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
#ifndef SERVER_H
#define SERVER_H

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <redox.hpp>

#include "http/server.h"
#include "http/httpserver.h"

#include "config.h"

#include "datastore.h"

/** @brief Content Directory Server (CDS) and API implementation. */
namespace cds {

/** @brief Content directory server */
class Server {
public:
    /** @brief The CDS Server CTOR. */
    Server ( const std::string& redis, /** @param redis redis host */
             const short port /** @param port the redis port. */ );

    /** @brief get configuration. */
    http::http_status config ( http::Request& request, http::Response& response );
    /** @brief rescan media directories. */
    http::http_status rescan ( http::Request& request, http::Response& response );
    /** @brief get content directory status. */
    http::http_status status ( http::Request& request, http::Response& response );

    /** @brief get node item. */
    http::http_status node ( http::Request& request, http::Response& response );
    /** @brief get node list. */
    http::http_status nodes ( http::Request& request, http::Response& response );
    /** @brief get file list. */
    http::http_status files ( http::Request& request, http::Response& response );


    /** @brief get sort criterias for the given node. */
    http::http_status sort ( http::Request& request, http::Response& response );
    /** @brief get the complete path for the given node. */
    http::http_status path ( http::Request& request, http::Response& response );
    /** @brief get the autocomplete items for the given string. */
    http::http_status sug ( http::Request& request, http::Response& response );

    http::http_status opds( http::Request& request, http::Response& response );

//    /** @brief get module node list */
//    http::http_status mod ( http::Request& request, http::Response& response );

    /** @brief get keywords list */
    http::http_status keywords ( http::Request& request, http::Response& response );

private:
    data::redis_ptr redis_;
    config_ptr config_;
    std::atomic<bool> rescanning_;
    std::unique_ptr< std::thread > scanner_thread_ = nullptr;
    redox::Subscriber sub_;

    /** @brief Rescan the content directory. */
    void rescan_ ( bool flush, std::function<void( std::error_code& )> fn );
};
}//namespace cds
#endif // SERVER_H
