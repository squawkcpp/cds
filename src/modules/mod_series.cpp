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

#include "image.h" //remove
#include "format.h"
#include "codec.h"

#include "http/httpclient.h"

#include "../_utils.h"
#include "../datastore.h"
#include "../utils/image.h"

namespace cds {
namespace mod {

void ModSeries::import ( data::redis_ptr redis, const config_ptr config ) {
    data::new_items( redis, data::NodeType::episode, [redis,config]( const std::string& key ) {
        try {
            data::node_t _file = data::node ( redis, key );
            //Get the track information
            av::Format _format ( _file[data::KEY_PATH] );

            if ( !!_format ) {
                spdlog::get ( LOGGER )->warn ( "Can not open serie file:{} ({})",
                                               _format.errc().message(),
                                               _file[data::KEY_PATH] );
            } else {

                //construct the episode name if empty
                std::string _episode_name = _file[data::TYPE_SERIE];
                _episode_name.append ( " S" ).append ( _file[PARAM_SEASON] )
                .append ( " E" ).append ( _file[data::TYPE_EPISODE] );
                auto codec = _format.find_codec ( av::CODEC_TYPE::VIDEO );
                auto _tmdb_id = import ( redis, config, key, _file[data::TYPE_SERIE] );
                auto _tmdb_result = tmdb_episode ( config->tmdb_key, _tmdb_id, _file[PARAM_SEASON], _file[data::TYPE_EPISODE] );
                auto _episodes = tmdb_parse_episode ( config, _tmdb_result );
                redis->command ( {data::REDIS_HMSET,  data::make_key_node ( key ),
                                data::KEY_NAME, ( _episodes[data::KEY_NAME].empty() ? _episode_name : _episodes[data::KEY_NAME] ),
                                PARAM_DATE, _episodes[PARAM_DATE],
                                PARAM_TMDB_ID, _episodes[PARAM_TMDB_ID],
                                PARAM_COMMENT, _episodes[PARAM_COMMENT],
                                PARAM_STILL_IMAGE, _episodes[PARAM_STILL_IMAGE],
                                KEY_PLAYTIME, std::to_string ( _format.playtime() ),
                                KEY_WIDTH, std::to_string ( codec->width() ),
                                KEY_HEIGHT, std::to_string ( codec->height() )
                });
                auto _last_write_time = boost::filesystem::last_write_time( data::get( redis, key, data::KEY_PATH ) );
                redis->command ( {data::REDIS_ZADD, data::make_key_list ( data::NodeType::episode ), std::to_string( _last_write_time ), key } );
            }
        } catch( std::error_code& code ) {
            spdlog::get ( LOGGER )->warn ( "error code in parse series: ({})", code.message() );
        } catch( ... ) {
            spdlog::get ( LOGGER )->warn ( "other error in parse series" );
        }
    } );
}

std::string ModSeries::import ( data::redis_ptr rdx, const config_ptr config, const std::string& serie_key, const std::string& serie ) {
    auto _clean_string = clean_string ( serie );
    redox::Command<int>& c = rdx->commandSync<int> ( { data::REDIS_EXISTS, data::make_key ( data::KEY_FS, data::hash ( _clean_string ) ) } );

    if ( c.ok() && c.reply() == 0 ) {
        auto _res_s = tmdb_get ( config->tmdb_key, _clean_string );
        auto _res = tmdb_parse ( _res_s );

        for ( auto& __res : _res ) {
            if ( clean_string ( __res[data::KEY_NAME] ) == _clean_string ) {
                std::string _poster_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, data::hash ( __res[PARAM_POSTER] ) ),
                            _backdrop_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, data::hash ( __res[PARAM_BACKDROP] ) );
                tmdb_fetch ( __res[PARAM_POSTER], _poster_path );
                image::Image image_meta_ ( _poster_path );
                image_meta_.scale ( 160, 160, fmt::format ( "{0}/tn_{1}.jpg", config->tmp_directory, data::hash ( __res[PARAM_POSTER] ) ) );
                tmdb_fetch ( __res[PARAM_BACKDROP], _backdrop_path );
                rdx->command ( {data::REDIS_HMSET,  data::make_key ( data::KEY_FS, data::hash ( _clean_string ) ),
                                data::KEY_CLASS, data::NodeType::str ( data::NodeType::serie ),
                                data::KEY_NAME, __res[data::KEY_NAME],
                                data::KEY_PARENT, "serie",
                                PARAM_TMDB_ID, __res[PARAM_TMDB_ID],
                                PARAM_COMMENT, __res[PARAM_COMMENT],
                                PARAM_DATE, __res["first_air_date"],
                                PARAM_BACKDROP, fmt::format ( "/img/{}.jpg", data::hash ( __res[PARAM_BACKDROP] ) ),
                                PARAM_POSTER, fmt::format ( "/img/{}.jpg", data::hash ( __res[PARAM_POSTER] ) ),
                                PARAM_THUMB, fmt::format ( "/img/tn_{}.jpg", data::hash ( __res[PARAM_POSTER] ) ),
                               } );
                break;
            } else {
                spdlog::get ( LOGGER )->warn ( "NO TMDB_RESULT: ({})", __res.size() );
                rdx->command ( {data::REDIS_HMSET,  data::make_key ( data::KEY_FS, data::hash ( _clean_string ) ),
                                data::KEY_CLASS, data::NodeType::str ( data::NodeType::serie ),
                                data::KEY_NAME, serie,
                               } );
            }
        }
        //TODO
        rdx->command ( {data::REDIS_ZADD, data::make_key_list ( data::NodeType::serie ), "0" /* TODO/*/, data::hash ( _clean_string ) } );
    }

    rdx->command ( { "SADD" /*TODO*/, data::make_key ( data::KEY_FS, data::hash ( _clean_string ), data::TYPE_SERIE ), serie_key } );
    //get the tmdb_id
    redox::Command<std::string>& _get_key = rdx->commandSync<std::string> (
    { "HGET" /*TODO*/, data::make_key ( data::KEY_FS, data::hash ( _clean_string ) ), PARAM_TMDB_ID } );

    if ( _get_key.ok() ) {
        return _get_key.reply();
    } else { throw http::http_status::NOT_FOUND; }
}

std::string ModSeries::tmdb_get ( const std::string& api_key, const std::string& name ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format ( "/3/search/tv?api_key={0}&query={1}", api_key, http::utils::UrlEscape::urlEncode ( name ) ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
}
std::string ModSeries::tmdb_episode ( const std::string& api_key, const std::string& serie_id, const std::string& season, const std::string& episode ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format ( "/3/tv/{}/season/{}/episode/{}?api_key={}", serie_id, season, episode, api_key ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
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
                _serie[PARAM_BACKDROP] = _obj->value.GetString();
            } else if ( strcmp ( _obj->name.GetString(), "overview" ) == 0 && _obj->value.IsString() ) {
                _serie[PARAM_COMMENT] = _obj->value.GetString();
            } else if ( strcmp ( _obj->name.GetString(), "poster_path" ) == 0 && _obj->value.IsString() ) {
                _serie[PARAM_POSTER] = _obj->value.GetString();
            } else if ( strcmp ( _obj->name.GetString(), "id" ) == 0 && _obj->value.IsInt() ) {
                _serie[PARAM_TMDB_ID] = std::to_string ( _obj->value.GetInt() );
            } else if ( strcmp ( _obj->name.GetString(), "first_air_date" ) == 0 && _obj->value.IsString() ) {
                struct tm _tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

                if ( !strptime ( _obj->value.GetString(), "%Y-%m-%d", &_tm ) ) {
                    spdlog::get ( LOGGER )->warn ( "tmdb date is invalid ({})", _obj->value.GetString() );
                } else {
                    time_t tmp_time_ = mktime ( &_tm );
                    _serie[PARAM_DATE] = tmp_time_;
                }
            } else if ( strcmp ( _obj->name.GetString(), data::KEY_NAME.c_str() ) == 0 && _obj->value.IsString() ) {
                _serie[data::KEY_NAME] = _obj->value.GetString();
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
        _episode[PARAM_DATE] = document["air_date"].GetString();
    }

    if ( document.HasMember ( "episode_number" ) && document["episode_number"].IsInt() ) {
        _episode[data::TYPE_EPISODE] = std::to_string ( document["episode_number"].GetInt() );
    }

    if ( document.HasMember ( data::KEY_NAME.c_str() ) && document[data::KEY_NAME.c_str()].IsString() ) {
        _episode[data::KEY_NAME] = document[data::KEY_NAME.c_str()].GetString();
    }

    if ( document.HasMember ( "overview" ) && document["overview"].IsString() ) {
        _episode[PARAM_COMMENT] = document["overview"].GetString();
    }

    if ( document.HasMember ( "id" ) && document["id"].IsInt() ) {
        _episode[PARAM_TMDB_ID] = std::to_string ( document["id"].GetInt() );
    }

    if ( document.HasMember ( "season_number" ) && document["season_number"].IsInt() ) {
        _episode[PARAM_SEASON] = std::to_string ( document["season_number"].GetInt() );
    }

    if ( document.HasMember ( "still_path" ) && document["still_path"].IsString() ) {
        auto _still_key = data::hash ( document["still_path"].GetString() );
        auto _still_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, _still_key );
        tmdb_fetch ( document["still_path"].GetString(), _still_path );
        _episode[PARAM_STILL_IMAGE] = fmt::format ( "/img/{}.jpg", _still_key );
    }

    return _episode;
}

void ModSeries::tmdb_fetch ( const std::string& uri, const std::string& path ) {
    std::ofstream _sstream ( path );
    http::get ( fmt::format ( "http://image.tmdb.org/t/p/original{}", uri ), _sstream );
}
}//namespace mod
}//namespace cds
