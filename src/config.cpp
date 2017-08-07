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
#include "config.h"

#include <arpa/inet.h>
#include <ifaddrs.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "_utils.h"

const char* config_category_t::name() const noexcept {
    return "cds::config";
}
std::error_condition config_category_t::default_error_condition ( int err ) const noexcept {
    if ( err == static_cast< int > ( LISTEN_ADDRESS ) )
    { return std::error_condition ( LISTEN_ADDRESS ); }

    if ( err == static_cast< int > ( HTTP_PORT ) )
    { return std::error_condition ( HTTP_PORT ); }

    if ( err == static_cast< int > ( TMP_DIRECTORY ) )
    { return std::error_condition ( TMP_DIRECTORY ); }

    if ( err == static_cast< int > ( TMDB_KEY ) )
    { return std::error_condition ( TMDB_KEY ); }

    if ( err == static_cast< int > ( AMAZON_ACCESS_KEY ) )
    { return std::error_condition ( AMAZON_ACCESS_KEY ); }

    if ( err == static_cast< int > ( AMAZON_KEY ) )
    { return std::error_condition ( AMAZON_KEY ); }

    if ( err == static_cast< int > ( MEDIA ) )
    { return std::error_condition ( MEDIA ); }

    return std::error_condition ( ERROR_CODES::UNKNOWN );
}
bool config_category_t::equivalent ( const std::error_code& code, int condition ) const noexcept {
    return *this == code.category() &&
           static_cast< int > ( default_error_condition ( code.value() ).value() ) == condition;
}
std::string config_category_t::message ( int err ) const {
    if ( err == static_cast< int > ( LISTEN_ADDRESS ) )
    { return "listen address not set."; }

    if ( err == static_cast< int > ( HTTP_PORT ) )
    { return "http port not set."; }

    if ( err == static_cast< int > ( TMP_DIRECTORY ) )
    { return "tmp directory not set."; }

    if ( err == static_cast< int > ( TMDB_KEY ) )
    { return "tmdb key not set"; }

    if ( err == static_cast< int > ( AMAZON_ACCESS_KEY ) )
    { return "amazon access key not set."; }

    if ( err == static_cast< int > ( AMAZON_KEY ) )
    { return "amazon key not set."; }

    if ( err == static_cast< int > ( MEDIA ) )
    { return "media directory is not set."; }

    return "unknown error condition.";
}

namespace cds {
inline std::string _get_ip() {
    std::string ip_ = "127.0.0.1";
    struct ifaddrs* ifAddrStruct = NULL;
    getifaddrs ( &ifAddrStruct );
    char addressBuffer[INET_ADDRSTRLEN];

    for ( struct ifaddrs* ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next ) {
        if ( ( ifa->ifa_addr )->sa_family == AF_INET ) {
            void* tmpAddrPtr = & ( ( struct sockaddr_in* ) ifa->ifa_addr )->sin_addr;
            inet_ntop ( AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN );

            if ( strcmp ( ifa->ifa_name, "lo" ) ) {
                ip_ = addressBuffer;
            }
        }
    }
    if ( ifAddrStruct != NULL ) {
        freeifaddrs ( ifAddrStruct );
    }
    return ip_;
}
std::vector< std::error_code > validate ( std::shared_ptr< Config > config ) {
    std::vector< std::error_code > _errors;

    //TODO get home directory

    if ( config->amazon_access_key.empty() )
    { _errors.push_back ( make_error_code ( AMAZON_ACCESS_KEY ) ); }

    if ( config->amazon_key.empty() )
    { _errors.push_back ( make_error_code ( AMAZON_KEY ) ); }

    if ( config->media.empty() )
    { _errors.push_back ( make_error_code ( MEDIA ) ); }

    if ( config->tmdb_key.empty() )
    { _errors.push_back ( make_error_code ( TMDB_KEY ) ); }

    if ( config->http_port.empty() )
    { config->http_port = "8081"; }

    if ( config->listen_address.empty() )
    { config->listen_address = _get_ip(); }

    if ( config->tmp_directory.empty() )
    { config->tmp_directory = "/var/tmp/squawk"; }

    return _errors;
}

std::string json ( std::shared_ptr< Config > config ) {
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();
    writer.String ( PARAM_LISTEN_ADDRESS );
    writer.String ( config->listen_address.c_str() );
    writer.String ( PARAM_HTTP_PORT );
    writer.String ( config->http_port.c_str() );
    writer.String ( PARAM_TMP_DIRECTORY );
    writer.String ( config->tmp_directory.c_str() );
    writer.String ( PARAM_TMDB_KEY );
    writer.String ( config->tmdb_key.c_str() );
    writer.String ( PARAM_AMAZON_ACCESS_KEY );
    writer.String ( config->amazon_access_key.c_str() );
    writer.String ( PARAM_AMAZON_KEY );
    writer.String ( config->amazon_key.c_str() );
    writer.String ( PARAM_DIRECTORY );
    writer.StartArray();
    for ( auto& __directory : config->media ) {
        writer.String ( __directory.c_str() );
    }
    writer.EndArray();
    writer.EndObject();
    return sb.GetString();
}
std::shared_ptr< Config > json ( const std::string& config ) {
    using namespace rapidjson;
    Document document;
    document.Parse ( config.c_str() );
    std::shared_ptr< Config > _cds = std::make_shared< Config >();

    if ( document.HasMember ( PARAM_LISTEN_ADDRESS ) && document[PARAM_LISTEN_ADDRESS].IsString() )
    { _cds->tmp_directory = document[PARAM_LISTEN_ADDRESS].GetString(); }

    if ( document.HasMember ( PARAM_HTTP_PORT ) && document[PARAM_HTTP_PORT].IsString() )
    { _cds->tmp_directory = document[PARAM_HTTP_PORT].GetString(); }

    if ( document.HasMember ( PARAM_TMP_DIRECTORY ) && document[PARAM_TMP_DIRECTORY].IsString() )
    { _cds->tmp_directory = document[PARAM_TMP_DIRECTORY].GetString(); }

    if ( document.HasMember ( PARAM_TMDB_KEY ) && document[PARAM_TMDB_KEY].IsString() )
    { _cds->tmdb_key = document[PARAM_TMDB_KEY].GetString(); }

    if ( document.HasMember ( PARAM_AMAZON_ACCESS_KEY ) && document[PARAM_AMAZON_ACCESS_KEY].IsString() )
    { _cds->amazon_access_key = document[PARAM_AMAZON_ACCESS_KEY].GetString(); }

    if ( document.HasMember ( PARAM_AMAZON_KEY ) && document[PARAM_AMAZON_KEY].IsString() )
    { _cds->amazon_key = document[PARAM_AMAZON_KEY].GetString(); }

    if ( document.HasMember ( PARAM_DIRECTORY ) &&
            document[PARAM_DIRECTORY].IsArray() ) {
        for ( auto& v : document[PARAM_DIRECTORY].GetArray() ) {
            _cds->media.push_back ( v.GetString() );
        }
    }
    return _cds;
}
}//namespace cds
