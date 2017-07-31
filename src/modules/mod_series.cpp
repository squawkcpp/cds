#include "mod_series.h"

#include <fstream>
#include <iostream>
#include <time.h>

#include "spdlog/spdlog.h"

#include "image.h"
#include "format.h"
#include "codec.h"

#include "http/httpclient.h"

namespace cds {
namespace mod {

void ModSeries::import( data::redox_ptr rdx, const CdsConfig& config ) {
    redox::Command<std::unordered_set< std::string > >& c =
        rdx->commandSync<std::unordered_set< std::string > >( { "SMEMBERS", "type:episode" } );

    if( c.ok() ) {
        for( const std::string& __c : c.reply() ) {
            std::map< std::string, std::string > _file = data::node( rdx, __c );

            //Get the track information
            av::Format _format( _file[PARAM_PATH] );
            if( !!_format )
            { throw _format.errc(); }

            //construct the episode name if empty
            std::string _episode_name = _file[PARAM_SERIE];
            _episode_name.append(" S").append( _file[PARAM_SEASON] )
            .append(" E").append( _file[PARAM_EPISODE] );

            auto codec = _format.find_codec( av::CODEC_TYPE::VIDEO );
            auto _tmdb_id = import( rdx, config, __c, _file[PARAM_SERIE] );
            auto _tmdb_result = tmdb_episode( config.tmdb_key, _tmdb_id, _file[PARAM_SEASON], _file[PARAM_EPISODE] );
            auto _episodes = tmdb_parse_episode( config, _tmdb_result );
            rdx->command( {"HMSET",  make_key( KEY_FS, __c ),
                           PARAM_NAME, ( _episodes[PARAM_NAME].empty() ? _episode_name : _episodes[PARAM_NAME] ),
                           PARAM_DATE, _episodes[PARAM_DATE],
                           PARAM_TMDB_ID, _episodes[PARAM_TMDB_ID],
                           PARAM_COMMENT, _episodes[PARAM_COMMENT],
                           PARAM_STILL_IMAGE, _episodes[PARAM_STILL_IMAGE],
                           KEY_PLAYTIME, std::to_string( _format.playtime() ),
                           KEY_WIDTH, std::to_string( codec->width() ),
                           KEY_HEIGHT, std::to_string( codec->height() )
                          } );
        }
    }
}

std::string ModSeries::import( data::redox_ptr rdx, const CdsConfig& config, const std::string& serie_key, const std::string& serie ) {
    auto _clean_string = clean_string( serie );
    redox::Command<int>& c = rdx->commandSync<int>( { "EXISTS", make_key( KEY_FS, hash( _clean_string ) ) } );
    if( c.ok() && c.reply() == 0 ) {
        auto _res_s = tmdb_get( config.tmdb_key, _clean_string );
        auto _res = tmdb_parse( _res_s );
        for( auto& __res : _res ) {
            if( clean_string( __res[PARAM_NAME] ) == _clean_string ) {
                std::string _poster_path = fmt::format( "{}/{}.jpg", config.tmp_directory, hash( __res[PARAM_POSTER] ) ),
                            _backdrop_path = fmt::format( "{}/{}.jpg", config.tmp_directory, hash( __res[PARAM_BACKDROP] ) );
                tmdb_fetch( __res[PARAM_POSTER], _poster_path );
                image::Image image_meta_ ( _poster_path );
                image_meta_.scale ( 160, 160, fmt::format( "{0}/tn_{1}.jpg", config.tmp_directory, hash( __res[PARAM_POSTER] ) ) );

                tmdb_fetch( __res[PARAM_BACKDROP], _backdrop_path );
                rdx->command( {"HMSET",  make_key( KEY_FS, hash( _clean_string ) ),
                               PARAM_CLASS, NodeType::name( NodeType::serie ),
                               PARAM_NAME, __res[PARAM_NAME],
                               PARAM_TMDB_ID, __res[PARAM_TMDB_ID],
                               PARAM_COMMENT, __res[PARAM_COMMENT],
                               PARAM_DATE, __res["first_air_date"],
                               PARAM_BACKDROP, fmt::format( "/cds/{}.jpg", hash( __res[PARAM_BACKDROP] ) ),
                               PARAM_POSTER, fmt::format( "/cds/{}.jpg", hash( __res[PARAM_POSTER] ) ),
                               PARAM_THUMB, fmt::format( "/cds/tn_{}.jpg", hash( __res[PARAM_POSTER] ) ),
                              });
                break;
            } else {
                spdlog::get(LOGGER)->warn( "NO TMDB_RESULT: ({})", __res.size() );
                rdx->command( {"HMSET",  make_key( KEY_FS, hash( _clean_string ) ),
                               PARAM_CLASS, NodeType::name( NodeType::serie ),
                               PARAM_NAME, serie,
                              });
            }
        }
        rdx->command( { "SADD", "fs:serie:list", hash( _clean_string ) } );
    }
    rdx->command( { "SADD", make_key( KEY_FS, hash( _clean_string ), PARAM_SERIE ), serie_key } );
    //get the tmdb_id
    redox::Command<std::string>& _get_key = rdx->commandSync<std::string>(
    { "HGET", make_key( KEY_FS, hash( _clean_string ) ), PARAM_TMDB_ID } );
    if( _get_key.ok() ) {
        return _get_key.reply();
    } else throw http::http_status::NOT_FOUND;
}

std::string ModSeries::tmdb_get ( const std::string& api_key, const std::string& name ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format( "/3/search/tv?api_key={0}&query={1}", api_key, http::utils::UrlEscape::urlEncode( name ) ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE(spdlog::get(LOGGER), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
}
std::string ModSeries::tmdb_episode ( const std::string& api_key, const std::string& serie_id, const std::string& season, const std::string& episode ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format( "/3/tv/{}/season/{}/episode/{}?api_key={}", serie_id, season, episode, api_key ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE(spdlog::get(LOGGER), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
}

std::vector < std::map<std::string, std::string > > ModSeries::tmdb_parse( const std::string& result ) {
    std::vector < std::map<std::string, std::string > > _series;

    rapidjson::Document document;
    document.Parse( result.c_str() );

    const auto& results = document[ "results" ];
    assert( results.IsArray() );
    for ( auto itr = results.Begin(); itr != results.End(); ++itr ) {
        std::map<std::string, std::string > _serie;
        for (auto _obj = itr->MemberBegin(); _obj != itr->MemberEnd(); ++_obj) {
            if( strncmp( _obj->name.GetString(), "backdrop_path", 13 ) == 0 && _obj->value.IsString() ) {
                _serie[PARAM_BACKDROP] = _obj->value.GetString();

            } else if( strncmp( _obj->name.GetString(), "overview", 8 ) == 0 && _obj->value.IsString() ) {
                _serie[PARAM_COMMENT] = _obj->value.GetString();

            } else if( strncmp( _obj->name.GetString(), "poster_path", 11 ) == 0 && _obj->value.IsString() ) {
                _serie[PARAM_POSTER] = _obj->value.GetString();

            } else if( strncmp( _obj->name.GetString(), "id", 11 ) == 0 && _obj->value.IsInt() ) {
                _serie[PARAM_TMDB_ID] = std::to_string( _obj->value.GetInt() );

            } else if( strncmp( _obj->name.GetString(), "first_air_date", 14 ) == 0 && _obj->value.IsString() ) {
                struct tm _tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
                if ( !strptime( _obj->value.GetString(), "%Y-%m-%d", &_tm ) ) {
                    spdlog::get(LOGGER)->warn( "tmdb date is invalid ({})", _obj->value.GetString() );
                } else {
                    time_t tmp_time_ = mktime ( &_tm );
                    _serie[PARAM_DATE] = tmp_time_;
                }
            } else if( strncmp( _obj->name.GetString(), PARAM_NAME.c_str(), PARAM_NAME.size() ) == 0 && _obj->value.IsString() ) {
                _serie[PARAM_NAME] = _obj->value.GetString();
            }
        }
        _series.push_back( _serie );
    }
    return _series;
}

std::map<std::string, std::string > ModSeries::tmdb_parse_episode( const CdsConfig& config, const std::string& result ) {
    rapidjson::Document document;
    document.Parse( result.c_str() );

    std::map<std::string, std::string > _episode;
    if( document.HasMember("air_date" ) && document["air_date"].IsString() ) {
        _episode[PARAM_DATE] = document["air_date"].GetString();

    }
    if( document.HasMember("episode_number" ) && document["episode_number"].IsInt() ) {
        _episode[PARAM_EPISODE] = std::to_string( document["episode_number"].GetInt() );

    }
    if( document.HasMember(PARAM_NAME.c_str()) && document[PARAM_NAME.c_str()].IsString() ) {
        _episode[PARAM_NAME] = document[PARAM_NAME.c_str()].GetString();

    }
    if( document.HasMember("overview") && document["overview"].IsString() ) {
        _episode[PARAM_COMMENT] = document["overview"].GetString();

    }
    if( document.HasMember("id" ) && document["id"].IsInt() ) {
        _episode[PARAM_TMDB_ID] = std::to_string( document["id"].GetInt() );

    }
    if( document.HasMember("season_number" ) && document["season_number"].IsInt() ) {
        _episode[PARAM_SEASON] = std::to_string( document["season_number"].GetInt() );

    }
    if( document.HasMember("still_path" ) && document["still_path"].IsString() ) {
        auto _still_key = hash( document["still_path"].GetString() );
        auto _still_path = fmt::format( "{}/{}.jpg", config.tmp_directory, _still_key );
        tmdb_fetch( document["still_path"].GetString(), _still_path );
        _episode[PARAM_STILL_IMAGE] = fmt::format( "/cds/{}.jpg", _still_key );
    }

    return _episode;
}

void ModSeries::tmdb_fetch( const std::string& uri, const std::string& path ) {
    std::ofstream _sstream( path );
    http::get ( fmt::format( "http://image.tmdb.org/t/p/original{}", uri ), _sstream );
}
}//namespace mod
}//namespace cds
