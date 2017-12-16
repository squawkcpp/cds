/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mod_series.h"

#include <fstream>
#include <iostream>
#include <time.h>

#include <boost/filesystem.hpp>
#include "spdlog/spdlog.h"
#include "rapidjson/document.h"

#include "format.h"
#include "codec.h"

#include "http/httpclient.h"

#include "../_utils.h"
#include "../datastore.h"
#include "../utils/image.h"

namespace cds {
namespace mod {

void ModSeries::import ( data::redis_ptr redis, const config_ptr config, const std::string& key ) {
    try {
        std::cout << "import serie: " << key << std::endl;
        //load the node from the database
        data::node_t _file = data::node ( redis, key );

        //Get the metadata from the file
        av::Format _format ( _file[param::PATH] );

        if ( !!_format ) {
            spdlog::get ( LOGGER )->warn ( "Can not open serie file:{} ({})",
                                           _format.errc().message(),
                                           _file[param::PATH] );
        } else {

            std::string _episode_name = _file[param::SERIE];
            std::string _tmdb_id;
            std::map<std::string, std::string > _episodes;
            //construct the episode name if empty
            if( _episode_name.empty() ) {
                _episode_name.append ( " S" ).append ( _file[param::SEASON] )
                .append ( " E" ).append ( _file[param::EPISODE] );
            } else {
                //load serie from tmdb
                sleep( SLEEP );
                _tmdb_id = import_serie ( redis, config, key, _episode_name );
                //load episode from tmdb
                sleep( SLEEP );
                auto _tmdb_result = tmdb_episode ( config->tmdb_key, _tmdb_id, _file[param::SEASON], _file[param::EPISODE] );
                if( std::get<0>(_tmdb_result) == http::http_status::OK ) {
                    _episodes = tmdb_parse_episode ( config, std::get<1>(_tmdb_result) );
                } else {
                    spdlog::get ( LOGGER )->warn ( "error get serie from tmdb: (tmdb_id:{}, s{}, e{} -> {})",
                            _tmdb_id, _file[param::SEASON], _file[param::EPISODE],
                            static_cast< int >( std::get<0>(_tmdb_result) ) );
                }
            }

            //save the episode to the database
            auto codec = _format.find_codec ( av::CODEC_TYPE::VIDEO );
            data::save ( redis, key, {
                { param::NAME, _episode_name },
                { param::PARENT, data::hash ( clean_string(_file[param::SERIE] ) ) },
                { param::DATE, _episodes[param::DATE] },
                { param::TMDB_ID, _episodes[param::TMDB_ID] },
                { param::COMMENT, _episodes[param::COMMENT] },
                { param::STILL_IMAGE, _episodes[param::STILL_IMAGE] },
                { param::PLAYTIME, std::to_string ( _format.playtime() ) },
                { param::WIDTH, std::to_string ( codec->width() ) },
                { param::HEIGHT, std::to_string ( codec->height() ) },
            });
            //create relations in the database
            auto _last_write_time = boost::filesystem::last_write_time( data::get( redis, key, param::PATH ) );
            data::add_types( redis, data::hash ( clean_string(_file[param::SERIE] ) ), key, static_cast< unsigned long >( _last_write_time ) );
        }
    } catch( std::error_code& code ) {
        spdlog::get ( LOGGER )->warn ( "error code in parse series: ({})", code.message() );
    } catch( ... ) {
        spdlog::get ( LOGGER )->warn ( "other error in parse series" );
    }
    std::cout << "<import serie" << std::endl;
}

std::string ModSeries::import_serie ( data::redis_ptr redis, const config_ptr config, const std::string& serie_key, const std::string& serie ) {
    auto _clean_string = clean_string ( serie );
    //check if the serie already exists
    redox::Command<int>& c = redis->commandSync<int> ( { redis::EXISTS, data::make_key_node ( data::hash ( _clean_string ) ) } );
    if ( c.ok() && c.reply() == 0 ) {
        //load and parse information from tmdb
        auto _res_s = tmdb_get ( config->tmdb_key, _clean_string );
        auto _res = tmdb_parse ( _res_s );
        //loop and find result
        if( !_res.empty() ) {
            for ( auto& __res : _res ) {
                std::cout << "result: " << __res[param::NAME] << std::endl;
                //TODO serie realtion has wrong key, is this executed?
                if ( clean_string ( __res[param::NAME] ) == _clean_string ) {
                    SPDLOG_DEBUG ( spdlog::get ( LOGGER ), "found serie: {}", _clean_string );
                    std::string _poster_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, data::hash ( __res[param::POSTER_PATH] ) ),
                                _backdrop_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, data::hash ( __res[param::BACKDROP_PATH] ) );

                    tmdb_fetch ( __res[param::POSTER_PATH], _poster_path );
                    utils::Image image_meta_ ( _poster_path );
                    image_meta_.scale ( config->tmp_directory, ECoverSizes::TN, data::hash ( __res[param::POSTER_PATH] ) );
                    image_meta_.scale ( config->tmp_directory, ECoverSizes::MED, data::hash ( __res[param::POSTER_PATH] ) );

                    tmdb_fetch ( __res[param::BACKDROP_PATH], _backdrop_path );

                    data::save ( redis,  data::hash ( _clean_string ), {
                        { param::CLASS, data::NodeType::str ( data::NodeType::serie ) },
                        { param::NAME, __res[param::NAME] },
                        { param::PARENT, param::SERIE },
                        { param::TMDB_ID, __res[param::TMDB_ID] },
                        { param::COMMENT, __res[param::COMMENT] },
                        { param::DATE, __res[param::DATE] },
                        { param::BACKDROP_PATH, fmt::format ( "/img/{}.jpg", data::hash ( __res[param::BACKDROP_PATH] ) ) },
                        { param::POSTER_PATH, fmt::format ( "/img/{}.jpg", data::hash ( __res[param::POSTER_PATH] ) ) },
                        { param::THUMB, fmt::format ( "/img/tn_{}.jpg", data::hash ( __res[param::POSTER_PATH] ) ) },
                        { param::MED, fmt::format ( "/img/med_{}.jpg", data::hash ( __res[param::POSTER_PATH] ) ) },
                    });
                    //add to serie list
                    data::add_nodes( redis, data::NodeType::serie, data::hash ( _clean_string /*TODO __res[param::POSTER_PATH]*/ ), data::time_millis() );
                    return( __res[param::TMDB_ID] );
                }
            }
        } else {
            std::cout << "NO TMDB_RESULT: " << serie << std::endl;
            spdlog::get ( LOGGER )->warn ( "NO TMDB_RESULT: ({})", serie );
            data::save ( redis, data::hash ( _clean_string ), {
                { param::CLASS, data::NodeType::str ( data::NodeType::serie ) },
                { param::NAME, serie }
            });
            data::add_nodes( redis, data::NodeType::serie, data::hash ( _clean_string ), data::time_millis() );
        }
    }
    //return existing serie key
    return data::get( redis, data::hash ( _clean_string ), param::TMDB_ID );
}

std::string ModSeries::tmdb_get ( const std::string& api_key, const std::string& name ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format ( "/3/search/tv?api_key={0}&query={1}", api_key, http::utils::UrlEscape::urlEncode ( name ) ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
}
std::tuple< http::http_status, std::string > ModSeries::tmdb_episode ( const std::string& api_key, const std::string& serie_id, const std::string& season, const std::string& episode ) {
    std::stringstream _sstream;
    auto _response = http::get(
        fmt::format ( "http://api.themoviedb.org/3/tv/{}/season/{}/episode/{}?api_key={}",
                      serie_id, season, episode, api_key ), _sstream );

    return( std::tuple< http::http_status, std::string >( _response.status(), _sstream.str() ) );
//    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
//    http::Request request_ ( fmt::format ( "/3/tv/{}/season/{}/episode/{}?api_key={}", serie_id, season, episode, api_key ) );
//    std::stringstream _sstream;
//    client_.get ( request_, _sstream );
//    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "TMDB RESULT: {}", _sstream.str() );
//    return _sstream.str();
}

std::vector < std::map<std::string, std::string > > ModSeries::tmdb_parse ( const std::string& result ) {
    std::vector < std::map<std::string, std::string > > _series;
    rapidjson::Document document;
    document.Parse ( result.c_str() );
    const auto& results = document[ "results" ];
    assert ( results.IsArray() );

    for ( auto itr = results.Begin(); itr != results.End(); ++itr ) {
        std::map<std::string, std::string > _serie;

        for ( auto _obj = itr->MemberBegin(); _obj != itr->MemberEnd(); ++_obj ) {
            if ( strcmp ( _obj->name.GetString(), "backdrop_path" ) == 0 && _obj->value.IsString() ) {
                _serie[param::BACKDROP_PATH] = _obj->value.GetString();
            } else if ( strcmp ( _obj->name.GetString(), "overview" ) == 0 && _obj->value.IsString() ) {
                _serie[param::COMMENT] = _obj->value.GetString();
            } else if ( strcmp ( _obj->name.GetString(), "poster_path" ) == 0 && _obj->value.IsString() ) {
                _serie[param::POSTER_PATH] = _obj->value.GetString();
            } else if ( strcmp ( _obj->name.GetString(), "id" ) == 0 && _obj->value.IsInt() ) {
                _serie[param::TMDB_ID] = std::to_string ( _obj->value.GetInt() );
            } else if ( strcmp ( _obj->name.GetString(), "first_air_date" ) == 0 && _obj->value.IsString() ) {
                struct tm _tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                if ( !strptime ( _obj->value.GetString(), "%Y-%m-%d", &_tm ) ) {
                    spdlog::get ( LOGGER )->debug ( "tmdb date is invalid ({})", _obj->value.GetString() );
                } else {
                    time_t tmp_time_ = mktime ( &_tm );
                    _serie[param::DATE] = std::to_string( tmp_time_ );
                }
            } else if ( strcmp ( _obj->name.GetString(), param::NAME.c_str() ) == 0 && _obj->value.IsString() ) {
                _serie[param::NAME] = _obj->value.GetString();
            }
        }
        _series.push_back ( _serie );
    }

    return _series;
}

std::map<std::string, std::string > ModSeries::tmdb_parse_episode ( const config_ptr config, const std::string& result ) {
    rapidjson::Document document;
    document.Parse ( result.c_str() );
    std::map<std::string, std::string > _episode;

    if ( document.HasMember ( "air_date" ) && document["air_date"].IsString() ) {
        _episode[param::DATE] = document["air_date"].GetString();
    }

    if ( document.HasMember ( "episode_number" ) && document["episode_number"].IsInt() ) {
        _episode[param::EPISODE] = std::to_string ( document["episode_number"].GetInt() );
    }

    if ( document.HasMember ( param::NAME.c_str() ) && document[param::NAME.c_str()].IsString() ) {
        _episode[param::NAME] = document[param::NAME.c_str()].GetString();
    }

    if ( document.HasMember ( "overview" ) && document["overview"].IsString() ) {
        _episode[param::COMMENT] = document["overview"].GetString();
    }

    if ( document.HasMember ( "id" ) && document["id"].IsInt() ) {
        _episode[param::TMDB_ID] = std::to_string ( document["id"].GetInt() );
    }

    if ( document.HasMember ( "season_number" ) && document["season_number"].IsInt() ) {
        _episode[param::SEASON] = std::to_string ( document["season_number"].GetInt() );
    }

    if ( document.HasMember ( "still_path" ) && document["still_path"].IsString() ) {
        auto _still_key = data::hash ( document["still_path"].GetString() );
        auto _still_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, _still_key );
        tmdb_fetch ( document["still_path"].GetString(), _still_path );
        _episode[param::STILL_IMAGE] = fmt::format ( "/img/{}.jpg", _still_key );
    }

    return _episode;
}

void ModSeries::tmdb_fetch ( const std::string& uri, const std::string& path ) {
    std::ofstream _sstream ( path );
    http::get ( fmt::format ( "http://image.tmdb.org/t/p/original{}", uri ), _sstream );
}
}//namespace mod
}//namespace cds
