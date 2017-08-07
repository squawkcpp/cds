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
#include <redox.hpp>

#include "config.h"
#include "datastore.h"

namespace cds {

/** @brief Scan media directory and add to store. */
class Scanner {
public:
    /** @brief import files */
    static void import_files ( data::redis_ptr redis, const config_ptr config );
private:
    Scanner() {}
    static void import_directory ( data::redis_ptr redis, magic_t& _magic, const std::string& parent_key, const std::string& path );
    static void import_video ( data::redis_ptr redis );
    static void import_images ( data::redis_ptr redis );
    static void import_books ( data::redis_ptr redis );
};
}//namespace cds
#endif // SCANNER_H
