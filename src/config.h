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
#ifndef CONFIG_H
#define CONFIG_H

#include <memory>
#include <string>
#include <system_error>
#include <vector>

/** @brief The cds ERROR_CODES enum */
enum ERROR_CODES {
    UNKNOWN, LISTEN_ADDRESS, HTTP_PORT, TMP_DIRECTORY, TMDB_KEY, AMAZON_ACCESS_KEY, AMAZON_KEY, MEDIA
};

///@cond DOC_INTERNAL
namespace std {
template<> struct is_error_condition_enum<ERROR_CODES> : public true_type {};
}//namespace std

class config_category_t : public std::error_category {
public:
    virtual const char* name() const noexcept;
    virtual std::error_condition default_error_condition ( int ev ) const noexcept;
    virtual bool equivalent ( const std::error_code& code, int condition ) const noexcept;
    virtual std::string message ( int ev ) const;
} static config_category;

inline std::error_condition make_error_condition ( ERROR_CODES e ) {
    return std::error_condition ( static_cast<int> ( e ), config_category );
}
///@endcond DOC_INTERNAL

namespace cds {
/** @brief cds configuration */
struct Config {
    std::string
        listen_address, http_port, tmp_directory,
        tmdb_key, amazon_access_key, amazon_key;
    std::vector< std::string > media;
};

/** @brief make cds config error code */
inline std::error_code make_error_code ( int error ) {
    switch ( error ) {
        case UNKNOWN:
        case LISTEN_ADDRESS:
        case HTTP_PORT:
        case TMP_DIRECTORY:
        case TMDB_KEY:
        case AMAZON_ACCESS_KEY:
        case AMAZON_KEY:
        case MEDIA:
            return std::error_code ( error, config_category );
    }

    return std::error_code ( error, std::generic_category() );
}

///@cond DOC_INTERNAL
std::string _get_ip();
///@endcond DOC_INTERNAL

/** @brief validate cds config*/
std::vector< std::error_code > validate ( std::shared_ptr< Config > config /** @param config configuration to validate */ );
/** @brief config to json */
std::string json ( std::shared_ptr< Config > config /** @param config configuration to output */ );
/** @brief json to config */
std::shared_ptr< Config > json ( const std::string& config /** @param config configuration string to parse */ );
/** @brief configuration ptr */
typedef std::shared_ptr< Config > config_ptr;
}//namespace cds
#endif // CONFIG_H
