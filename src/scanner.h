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
#ifndef SCANNER_H
#define SCANNER_H

#include <string>

#include <magic.h>

#include "config.h"
#include "datastore.h"

#include "gtest/gtest_prod.h"

namespace cds {

/** @brief Scan media directory and add to store. */
class Scanner {
public:
    /** @brief import files */
    static void import_files ( data::redis_ptr redis /** @param redis pointer to redis database object. */,
                               const config_ptr config /** @param config pointer to configuration object. */ );

    static void flush( data::redis_ptr redis );

private:
    /** @brief hide constructor. */
    Scanner() {}
    static const std::array< std::regex, 3 > _disc_patterns;

    static void import_directory ( data::redis_ptr redis, magic_t& _magic, const std::string& parent_key, const std::string& path );
    static void import_video ( data::redis_ptr redis );
    static void import_images ( data::redis_ptr redis );
    static void import_books ( data::redis_ptr redis );
    static void new_item( data::redis_ptr redis /** @param redis redis database pointer. */,
                          const std::string& parent /** @param parent parent path for audiofiles. */,
                          const std::string& key /** @param key key of the node. */,
                          const data::NodeType::Enum type /** @param type type of the node. */ );
    static void sweep ( data::redis_ptr redis, const std::string& key );
    static void sweep_ref ( data::redis_ptr redis /** @param redis redis database pointer. */,
                            data::NodeType::Enum type /** param type the node type */ );
    static bool timestamp( data::redis_ptr redis /** @param redis redis database pointer. */,
                           const std::string& key /** @param key key of the node. */,
                           unsigned long timestamp /** @param timestamp the last write timestamp. */ );
    FRIEND_TEST( ScannerTest, remove_disc );
    static std::string remove_disc( const std::string& path );

    static void new_items ( data::redis_ptr redis, const config_ptr config, data::NodeType::Enum type,
                            std::function< void(data::redis_ptr, const config_ptr, const std::string&) > fn );

    static void save_folder ( data::redis_ptr redis /** @param redis redis database pointer. */,
                              const std::string& key /** @param path path of the node. */,
                              const std::string& name /** @param name name of the node. */,
                              const std::string& parent /** @param parent parent key. */ );

    static void search_index ( data::redis_ptr redis /** @param redis redis database pointer. */ );
};
}//namespace cds
#endif // SCANNER_H
