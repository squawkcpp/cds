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

#include "libavcpp/format.h"
#include "libavcpp/codec.h"

#include "http/httpclient.h"

#include "../_utils.h"
#include "../datastore.h"
#include "../utils/image.h"

namespace cds {
namespace mod {
void ModMovies::import ( data::redis_ptr redis, const config_ptr config, const std::string& key ) {
    try {
        data::node_t _node = data::node ( redis, key );

        //Get the track information
        av::Format _format ( _node[param::PATH] );

        if ( !!_format ) {
            spdlog::get ( LOGGER )->warn ( "Can not open movie file:{} ({})",
                                           _format.errc().message(),
                                           _node[param::PATH] );
        } else {

            //get the movie metadata from tmdb
            auto _clean_string = clean_string ( remove_skip_list ( trash_words, _node[param::NAME] ) );

            sleep ( SLEEP );
            auto _tmdb_result = tmdb_search( config->tmdb_key, _clean_string );
            auto _tmdb_movie_list = tmdb_parse( _tmdb_result );
            if( !_tmdb_movie_list.empty() ) {
                bool _found = false;
                std::string _movie_id;
                for( auto& __movie : _tmdb_movie_list ) {
                    if( __movie.at( param::TITLE ) == _clean_string ) {
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

                int _width = 0, _height = 0, _playtime = 0;
                auto codec = _format.find_codec ( av::CODEC_TYPE::VIDEO );
//TODO                    _width = codec->width();
//                    _height = codec->height();
//                    _playtime = _format.playtime();

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

                data::save ( redis, key, {
                    { param::WIDTH, std::to_string ( _width ) },
                    { param::HEIGHT, std::to_string ( _height ) },
                    { param::PLAYTIME, std::to_string ( _playtime ) },
                    { param::BACKDROP_PATH, fmt::format ( "/img/{}.jpg", _backdrop_key ) },
                    { param::HOMEPAGE, _tmdb_movie.homepage },
                    { param::TMDB_ID, _tmdb_movie.id },
                    { param::IMDB_ID, _tmdb_movie.imdb_id },
                    { param::LANGUAGE, _tmdb_movie.original_language },
                    { param::TITLE, _tmdb_movie.original_title },
                    { param::COMMENT, _tmdb_movie.overview },
                    { param::POSTER_PATH, fmt::format ( "/img/{}.jpg", _poster_key ) },
                    { param::DATE, _tmdb_movie.release_date },
                    { param::PLAYTIME, _tmdb_movie.runtime },
                    { "tagline", _tmdb_movie.tagline }, //TODO what is the content of the tagline
                    { param::TITLE, _tmdb_movie.title },
                    { param::THUMB, fmt::format ( "/img/tn_{}.jpg", key ) },
                    { param::MED, fmt::format ( "/img/med_{}.jpg", key ) },
                });

                //save genre tags
                for( auto& __tag : _tmdb_movie.genres ) {
                    data::add_tag( redis, data::NodeType::movie, param::GENRE, __tag, key, 0 );
                }

            } else { //TODO get cover from movie
                spdlog::get ( LOGGER )->warn ( "NO TMDB_RESULT: ({})", _clean_string );
            }
            auto _last_write_time = boost::filesystem::last_write_time( data::get( redis, key, param::PATH ) );
            data::add_nodes( redis, data::NodeType::movie, key, static_cast<unsigned long>( _last_write_time ) );
        }
    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception mod movies." );
    }
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
            } else if ( strcmp ( _obj->name.GetString(), param::TITLE.c_str() ) == 0 && _obj->value.IsString() ) {
                _movie[param::TITLE] = _obj->value.GetString();
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

    if ( document.HasMember ( param::BACKDROP_PATH.c_str() ) && document[param::BACKDROP_PATH.c_str()].IsString() ) {
        _movie.backdrop_path = document[param::BACKDROP_PATH.c_str()].GetString();
    }
    if ( document.HasMember ( param::HOMEPAGE.c_str() ) && document[param::HOMEPAGE.c_str()].IsString() ) {
        _movie.homepage = document[param::HOMEPAGE.c_str()].GetString();
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
    if ( document.HasMember ( param::TITLE.c_str() ) && document[param::TITLE.c_str()].IsString() ) {
        _movie.title = document[param::TITLE.c_str()].GetString();
    }
    if ( document.HasMember ( key::GENRES.c_str() ) && document[key::GENRES.c_str()].IsArray() ) {
        for( auto& __genre : document[key::GENRES.c_str()].GetArray() ) {
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
