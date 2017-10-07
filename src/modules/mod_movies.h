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
#ifndef MOD_MOVIES_H
#define MOD_MOVIES_H

#include <string>

#include "../datastore.h"
#include "../config.h"

#include "gtest/gtest_prod.h"

namespace cds {
namespace mod {

struct TmdbMovie {

     std::string backdrop_path;
     std::vector< std::string > genres;
     std::string homepage;
     std::string id;
     std::string imdb_id;
     std::string original_language;
     std::string original_title;
     std::string overview;
     std::string poster_path;
     std::string release_date;
     std::string runtime;
     std::string tagline;
     std::string title;

     friend std::ostream& operator <<(std::ostream &os,const TmdbMovie &obj) {
         os <<  "TmdbMovie{" <<
                "backdrop_path:" << obj.backdrop_path <<
                ", homepage" << obj.homepage <<
                ", id:" << obj.homepage <<
                ", imdb_id:" << obj.imdb_id <<
                ", original_language:" << obj.original_language <<
                ", original_title:" << obj.original_title <<
                ", overview:" << obj.overview <<
                ", poster_path:" << obj.poster_path <<
                ", release_date:" << obj.release_date <<
                ", runtime:" << obj.runtime <<
                ", tagline:" << obj.tagline <<
                ", title:" << obj.title;
        os << ", genres[";
        bool is_first = true;
         for( const auto &iter : obj.genres ) {
             if( is_first ) is_first=false; else os << ", ";
             os << iter;
         }
         os << "]}";
         return os;
     }
};

class ModMovies {
public:
    static void import ( data::redis_ptr redis, const config_ptr config, const std::string& key );

private:
    ModMovies() {}
    static std::string tmdb_search ( const std::string& api_key, const std::string& name );
    static std::string tmdb_movie ( const std::string& api_key, const std::string& movie_id );
    FRIEND_TEST ( ModMoviesTest, tmdb_parse );
    static std::vector < std::map<std::string, std::string > > tmdb_parse ( const std::string& result );
    FRIEND_TEST ( ModMoviesTest, tmdb_parse_movie );
    static TmdbMovie tmdb_parse_movie ( const std::string& result );
    static void tmdb_fetch ( const std::string& uri, const std::string& path );
};
}//namespace mod
}//namespace cds
#endif // MOD_MOVIES_H
