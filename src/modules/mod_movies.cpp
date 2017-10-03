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
#include "mod_movies.h"

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
void ModMovies::import ( data::redis_ptr redis, const config_ptr config ) {

    data::new_items( redis, data::NodeType::movie, [redis,config]( const std::string& key ) {
        data::node_t _node = data::node ( redis, key );

        //Get the track information
        av::Format _format ( _node[data::KEY_PATH] );

        if ( !!_format ) {
            spdlog::get ( LOGGER )->warn ( "Can not open movie file:{} ({})",
                                           _format.errc().message(),
                                           _node[data::KEY_PATH] );
        } else {

            //get the movie metadata from tmdb
            auto _clean_string = clean_string ( remove_skip_list ( trash_words, _node[data::KEY_NAME] ) );

            sleep ( SLEEP );
            std::cout << "search movie: " << _clean_string << std::endl;
            auto _tmdb_result = tmdb_search( config->tmdb_key, _clean_string );
            auto _tmdb_movie_list = tmdb_parse( _tmdb_result );
            if( !_tmdb_movie_list.empty() ) {
                bool _found = false;
                std::string _movie_id;
                for( auto& __movie : _tmdb_movie_list ) {
                    if( __movie.at( data::KEY_TITLE ) == _clean_string ) {
                        _found = true;
                        _movie_id = __movie.at( "id" );
                        break;
                    }
                }
                if( !_found )
                { _movie_id = _tmdb_movie_list.at( 0 ).at( "id" ); }

                //set the values from tmdb to the video item.
                auto _tmdb_movie_result = tmdb_movie( config->tmdb_key, _movie_id );
                auto _tmdb_movie = tmdb_parse_movie( _tmdb_movie_result );

                auto codec = _format.find_codec ( av::CODEC_TYPE::VIDEO );

                //get backdrop image
                auto _backdrop_key = data::hash ( _tmdb_movie.backdrop_path );
                auto _backdrop_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, _backdrop_key );
                tmdb_fetch ( _tmdb_movie.backdrop_path, _backdrop_path );
                //get poster image
                auto _poster_key = data::hash ( _tmdb_movie.poster_path );
                auto _poster_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, _poster_key );
                tmdb_fetch ( _tmdb_movie.poster_path, _poster_path );

                utils::Image image_meta_ ( _poster_path );
                image_meta_.scale ( config->tmp_directory, ECoverSizes::TN, key );
                image_meta_.scale ( config->tmp_directory, ECoverSizes::MED, key );

                redis->command ( { data::REDIS_HMSET,  data::make_key_node ( key ),
                    KEY_BITRATE, std::to_string ( codec->bitrate() ),
                    KEY_BPS, std::to_string ( codec->bits_per_sample() ),
                    KEY_CHANNELS, std::to_string ( codec->channels() ),
                    KEY_WIDTH, std::to_string ( codec->width() ),
                    KEY_HEIGHT, std::to_string ( codec->height() ),
                    "backdrop_path", fmt::format ( "/img/{}.jpg", _backdrop_key ),
                    "homepage", _tmdb_movie.homepage,
                    "id", _tmdb_movie.id,
                    "imdb_id", _tmdb_movie.imdb_id,
                    "original_language", _tmdb_movie.original_language,
                    "original_title", _tmdb_movie.original_title,
                    "overview", _tmdb_movie.overview,
                    "poster_path", fmt::format ( "/img/{}.jpg", _poster_key ),
                    "release_date", _tmdb_movie.release_date,
                    "runtime", _tmdb_movie.runtime,
                    "tagline", _tmdb_movie.tagline,
                    data::KEY_TITLE, _tmdb_movie.title,
                    PARAM_THUMB, fmt::format ( "/img/tn_{}.jpg", key ),
                });

                //save genre tags
                for( auto& __tag : _tmdb_movie.genres ) {
                    data::add_tag( redis, "genre", __tag, data::NodeType::movie, key, 0 );
                }

            } else { //TODO get cover from movie
                std::cout << "no tmdb result: " << _clean_string << std::endl;
            }
            auto _last_write_time = boost::filesystem::last_write_time( data::get( redis, key, data::KEY_PATH ) );
            redis->command ( {data::REDIS_ZADD, data::make_key_list ( data::NodeType::movie ), std::to_string( _last_write_time ), key } );
        }
    });
}
std::string ModMovies::tmdb_search ( const std::string& api_key, const std::string& name ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format ( "/3/search/movie?api_key={0}&query={1}&include_adult=true", api_key, http::utils::UrlEscape::urlEncode ( name ) ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
}
std::string ModMovies::tmdb_movie ( const std::string& api_key, const std::string& movie_id ) {
    http::HttpClient< http::Http>  client_ ( "api.themoviedb.org", "http" );
    http::Request request_ ( fmt::format ( "/3/movie/{}?api_key={}", movie_id, api_key ) );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "TMDB RESULT: {}", _sstream.str() );
    return _sstream.str();
}
std::vector < std::map<std::string, std::string > > ModMovies::tmdb_parse ( const std::string& result ) {
    std::vector < std::map<std::string, std::string > > _movies;
    rapidjson::Document document;
    document.Parse ( result.c_str() );
    const auto& results = document[ "results" ];
    assert ( results.IsArray() );

    for ( auto itr = results.Begin(); itr != results.End(); ++itr ) {
        std::map<std::string, std::string > _movie;

        for ( auto _obj = itr->MemberBegin(); _obj != itr->MemberEnd(); ++_obj ) {
            if (  strcmp ( _obj->name.GetString(), "id" ) == 0 && _obj->value.IsInt() ) {
                _movie["id"] = std::to_string( _obj->value.GetInt() );
            } else if ( strcmp ( _obj->name.GetString(), data::KEY_TITLE.c_str() ) == 0 && _obj->value.IsString() ) {
                _movie[data::KEY_TITLE] = _obj->value.GetString();
            }
        }

        _movies.push_back ( _movie );
    }

    return _movies;
}
TmdbMovie ModMovies::tmdb_parse_movie ( const std::string& result ) {
    rapidjson::Document document;
    document.Parse ( result.c_str() );
    TmdbMovie _movie;

    if ( document.HasMember ( "backdrop_path" ) && document["backdrop_path"].IsString() ) {
        _movie.backdrop_path = document["backdrop_path"].GetString();
    }
    if ( document.HasMember ( "homepage" ) && document["homepage"].IsString() ) {
        _movie.homepage = document["homepage"].GetString();
    }
    if ( document.HasMember ( "id" ) && document["id"].IsInt() ) {
        _movie.id = std::to_string( document["id"].GetInt() );
    }
    if ( document.HasMember ( "imdb_id" ) && document["imdb_id"].IsString() ) {
        _movie.imdb_id = document["imdb_id"].GetString();
    }
    if ( document.HasMember ( "original_language" ) && document["original_language"].IsString() ) {
        _movie.original_language = document["original_language"].GetString();
    }
    if ( document.HasMember ( "original_title" ) && document["original_title"].IsString() ) {
        _movie.original_title = document["original_title"].GetString();
    }
    if ( document.HasMember ( "overview" ) && document["overview"].IsString() ) {
        _movie.overview = document["overview"].GetString();
    }
    if ( document.HasMember ( "poster_path" ) && document["poster_path"].IsString() ) {
        _movie.poster_path = document["poster_path"].GetString();
    }
    if ( document.HasMember ( "release_date" ) && document["release_date"].IsString() ) {
        _movie.release_date = document["release_date"].GetString();
    }
    if ( document.HasMember ( "runtime" ) && document["runtime"].IsInt() ) {
        _movie.runtime = std::to_string( document["runtime"].GetInt() );
    }
    if ( document.HasMember ( "tagline" ) && document["tagline"].IsString() ) {
        _movie.tagline = document["tagline"].GetString();
    }
    if ( document.HasMember ( data::KEY_TITLE.c_str() ) && document[data::KEY_TITLE.c_str()].IsString() ) {
        _movie.title = document[data::KEY_TITLE.c_str()].GetString();
    }
    if ( document.HasMember ( data::KEY_GENRES.c_str() ) && document[data::KEY_GENRES.c_str()].IsArray() ) {
        for( auto& __genre : document[data::KEY_GENRES.c_str()].GetArray() ) {
            if ( __genre.HasMember ( "name" ) && __genre["name"].IsString() ) {
                _movie.genres.push_back( __genre["name"].GetString() );
            }
        }
    }
    return _movie;
}
void ModMovies::tmdb_fetch ( const std::string& uri, const std::string& path ) {
    std::ofstream _sstream ( path );
    http::get ( fmt::format ( "http://image.tmdb.org/t/p/original{}", uri ), _sstream );
}
}//namespace mod
}//namespace cds
