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
#ifndef MOD_ALBUMS_H
#define MOD_ALBUMS_H

#include <string>

#include "../config.h"
#include "../datastore.h"

#include "gtest/gtest_prod.h"

namespace cds {
namespace mod {

class ModAlbums {
public:
    static void import ( data::redis_ptr rdx, const config_ptr config );
private:
    static void import ( data::redis_ptr rdx,
                         const std::string& key, std::map< NodeType::Enum, std::vector< data::node_t > >& files );
    static void import ( data::redis_ptr rdx,
                         const std::string& album_key, const std::string& artist );

    FRIEND_TEST ( MusicbrainzTest, parse_artist_mbid );
    static std::string mbid_parse ( const std::string& artist );
    FRIEND_TEST ( MusicbrainzTest, parse_artist_metadata );
    static std::map<std::string, std::string> get_artist_metadata ( const std::string& metadata );
    static std::string artist_meta_get ( const std::string& mbid );
    static std::string mbid_get ( const std::string& name );
    ModAlbums() {}
};
}//namespace mod
}//namespace cds
#endif // MOD_ALBUMS_H
