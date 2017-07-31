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

/** @brief The CDS Server class. */
class Server {
public:
    /** @brief The CDS Server CTOR. */
    Server( std::shared_ptr< http::Server< http::HttpServer > > web_server, /** @param web_server the server to attach the API uri's. */
            const std::string& redis, /** @param redis redis host */
            const short port, /** @param port the redis port. */
            const std::string& tmp_directory, /** @param tmp_directory temporary directorie for the images */
            const std::string& tmdb_key, /** @param tmdb_key tmdb API key */
            const std::string& amazon_access_key, /** @param amazon_access_key amazon access key */
            const std::string& amazon_key, /** @param tmdb_key amazon API key */
            const std::vector< std::string >& media /** @param media directories to use for the content directory. */ );

    /** @brief Rescan the content directory. */
    void rescan ( CdsConfig& c /** @param c the configuration to use. */, bool flush /** @param flush flush data before scan. */ );
private:
    data::redox_ptr redis_;
    const std::string redis_ip_;
    const short redis_port_;
    std::mutex rescan_mutex_;
    redox::Subscriber sub_;
    CdsConfig config_;
};
}//namespace cds
#endif // SERVER_H
