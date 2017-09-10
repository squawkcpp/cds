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
#ifndef FILEMETAREGEX_H
#define FILEMETAREGEX_H

#include <map>
#include <regex>
#include <string>
#include <vector>

#include "datastore.h"

namespace cds {

/** @brief The RuleItem struct */
struct RuleItem {
    data::NodeType::Enum type;
    std::regex regex;
    std::vector< std::string > names;
};

/** @brief The FileMetaRegex class */
class FileMetaRegex {
public:
    /** @brief parse filename and store result in metadata map */
    static data::NodeType::Enum parse ( const std::string& mime_type /** @param mime_type mime-type of the item. */,
                                  const std::string& path /** @param path path of the item. */,
                                  data::node_t& metadata /** @param metadata store the meta data. */ );
private:
    FileMetaRegex() {}
    static std::map< std::string, std::vector< RuleItem > > rules_;
};
}//namespace cds
#endif // FILEMETAREGEX_H
