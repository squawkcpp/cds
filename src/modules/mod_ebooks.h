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
#ifndef MOD_EBOOKS_H
#define MOD_EBOOKS_H

#include <string>

#include "../config.h"
#include "../datastore.h"

namespace cds {
namespace mod {

class ModEbooks {
public:
    static void import ( data::redis_ptr rdx, const config_ptr config, const std::string& key  );
private:
    ModEbooks();
    static const std::string isbn_regex;
    static const std::string isbn_number_only_regex;
    static std::string parsePdf ( const std::string& filename );
    static const std::string find_isbn ( const std::string& body );
    static const std::string remove_special_characters ( const std::string& body );
};
}//namespace mod
}//namespace cds
#endif // MOD_EBOOKS_H
