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

namespace cds {
namespace mod {

/** @brief Audiofile import module. */
class ModAlbums {
public:
    /** @brief import */
    static void import ( data::redis_ptr redis /** @param redis redis database pointer. */,
                         const config_ptr config /** @param config cds configuration pointer. */,
                         const std::string& key );
private:
    static void import_files ( data::redis_ptr rdx,
                         const std::string& key, std::map< data::NodeType::Enum, std::vector< data::node_t > >& files );
    static void import_artist ( data::redis_ptr rdx,
                         const std::string& album_key, const std::string& artist );
    ModAlbums() {}
};
}//namespace mod
}//namespace cds
#endif // MOD_ALBUMS_H
