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
#ifndef MOD_SERIES_H
#define MOD_SERIES_H

#include <string>
#include <tuple>

#include "../config.h"
#include "../datastore.h"

#include "http/constant.h"

namespace cds {
namespace mod {

/** @brief TV Serie import module. */
class ModSeries {
public:
    /** @brief import */
    static void import ( data::redis_ptr redis /** @param redis redis database pointer. */,
                         const config_ptr config /** @param config cds configuration pointer. */,
                         const std::string& key /** @param key the file database key **/ );

private:
    ModSeries() {}

    static std::string import_serie ( data::redis_ptr rdx, const config_ptr config, const std::string& serie_key, const std::string& serie );

    static std::string tmdb_get ( const std::string& api_key, const std::string& name );
    static std::tuple< http::http_status, std::string > tmdb_episode ( const std::string& api_key, const std::string& serie_id, const std::string& season, const std::string& episode );
    static std::vector < std::map<std::string, std::string > > tmdb_parse ( const std::string& result );
    static std::map<std::string, std::string> tmdb_parse_episode ( const config_ptr config, const std::string& result );
    static void tmdb_fetch ( const std::string& uri, const std::string& path );
};
}//namespace mod
}//namespace cds
#endif // MOD_SERIES_H
