/*
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <iostream>
#include <string>
#include <vector>

#include "cxxopts.hpp"
#include "spdlog/spdlog.h"
#include <boost/filesystem.hpp>

#include "http/mod/exec.h"
#include "http/mod/file.h"
#include "http/mod/match.h"
#include "http/mod/method.h"

#include "src/_utils.h"
#include "src/config.h"
#include "src/datastore.h"
#include "src/server.h"

using namespace std::placeholders;

struct Container {
    cds::config_ptr config;
    data::redis_ptr redox;
    std::shared_ptr< cds::Server > server;
    std::shared_ptr< http::Server< http::HttpServer > > www;
};

void signalHandler( int signum ) {
    std::cout << "CDS:Interrupt signal (" << signum << ") received.\n";
    exit( signum );
}

int main(int argc, char* argv[]) {

    cxxopts::Options options("cds", "Squawk content directory server.");
    options.add_options()
        ( cds::PARAM_DIRECTORY, "path to the directory with the mediafiles. Multiple entries will result in a list containing all directories.",
          cxxopts::value<std::vector<std::string>>(), "PATH" )
        ( cds::PARAM_LISTEN_ADDRESS, "API Webserver IP-Adress to bind to.", cxxopts::value<std::string>(), "IP" )
        ( cds::PARAM_HTTP_PORT, "API Webserver IP Port to bind to.", cxxopts::value<std::string>(), "PORT" )
        ( cds::PARAM_TMP_DIRECTORY, "temporary folder for the thumbnails.", cxxopts::value<std::string>(), "PATH" )
        ( cds::PARAM_TMDB_KEY, "API key for tmdb.", cxxopts::value<std::string>(), "KEY" )
        ( cds::PARAM_AMAZON_ACCESS_KEY, "Access key for amazon.", cxxopts::value<std::string>(), "KEY" )
        ( cds::PARAM_AMAZON_KEY, "API key for amazon.", cxxopts::value<std::string>(), "KEY" )
        ( cds::PARAM_REDIS, "Redis Database (default: localhost)", cxxopts::value<std::string>()->default_value("localhost"), "HOST" )
        ( cds::PARAM_REDIS_PORT, "Redis Database port (default: 6379)", cxxopts::value<std::string>()->default_value("6379"), "PORT" )
        ( "help", "Print help")
      ;
    options.parse(argc, argv);

    if( options.count( "help" ) ) {
         std::cout << options.help({"", "Group"}) << std::endl;
         exit(0);
    }

    Container _container;

    auto& _redis_server = options[cds::PARAM_REDIS].as<std::string>();
    auto& _redis_port = options[cds::PARAM_REDIS_PORT].as<std::string>();
    _container.redox = data::make_connection( _redis_server, std::stoi( _redis_port ) );
    //load config from database
    if( data::config_exists( _container.redox ) ) {
        _container.config = cds::json( data::config( _container.redox ) );
    } else _container.config = std::make_shared< cds::Config >();

    if ( options.count( cds::PARAM_DIRECTORY ) ) {
        _container.config->media.clear();
        auto& ff = options[cds::PARAM_DIRECTORY].as<std::vector<std::string>>();
        for (const auto& f : ff) {
            if( boost::filesystem::is_directory( f ) ) {
                _container.config->media.push_back( f );
            } else {
                std::cout << "Path is not a directory. " << f << std::endl;
                return 1;
            }
        }
    }

    if ( options.count( cds::PARAM_TMP_DIRECTORY ) )
    { _container.config->tmp_directory =  options[cds::PARAM_TMP_DIRECTORY].as<std::string>(); }
    if ( options.count( cds::PARAM_TMDB_KEY ) )
    { _container.config->tmdb_key =  options[cds::PARAM_TMDB_KEY].as<std::string>(); }
    if ( options.count( cds::PARAM_AMAZON_ACCESS_KEY ) )
    { _container.config->amazon_access_key =  options[cds::PARAM_AMAZON_ACCESS_KEY].as<std::string>(); }
    if ( options.count( cds::PARAM_AMAZON_KEY ) )
    { _container.config->amazon_key =  options[cds::PARAM_AMAZON_KEY].as<std::string>(); }
    if ( options.count( cds::PARAM_LISTEN_ADDRESS ) )
    { _container.config->listen_address =  options[cds::PARAM_LISTEN_ADDRESS].as<std::string>(); }
    if ( options.count( cds::PARAM_HTTP_PORT ) )
    { _container.config->http_port = options[cds::PARAM_HTTP_PORT].as<std::string>(); }

    //store configuration
    auto _config_e = cds::validate( _container.config );
    if( !_config_e.empty() ) {
        for( auto& e : _config_e )
            std::cerr << "*" << e.message() << "\n";
        std::cerr << std::endl;
        return -1;
    }
    data::config( _container.redox, cds::json( _container.config ) );

    /* setup logger */
#ifdef DEBUG
    spdlog::set_level( spdlog::level::trace );
    auto console = spdlog::stdout_color_mt( cds::LOGGER );
#else
    spdlog::set_level( spdlog::level::info );
    auto console = spdlog::rotating_logger_mt( cds::LOGGER, "/var/log/cds.log" /** TODO make configurable */, 1048576 * 5, 3 );
#endif

    console->info( "Start content directory server ({}:{})",
        _container.config->listen_address, _container.config->http_port );

    /* Setup and start the server **/
    _container.server = std::make_shared< cds::Server >( _redis_server, std::stoi( _redis_port ) );
    _container.www = std::shared_ptr< http::Server< http::HttpServer > >(
        new http::Server< http::HttpServer >( _container.config->listen_address, _container.config->http_port ) );

    _container.www->bind( http::mod::Match<>( "^\\/config$" ),
        http::mod::Exec( std::bind( &cds::Server::config, _container.server, _1, _2 ) ),
        http::mod::Http() );

    _container.www->bind( http::mod::Match<>( "^\\/rescan$" ),
        http::mod::Exec( std::bind( &cds::Server::rescan, _container.server, _1, _2 ) ),
        http::mod::Http() );

    _container.www->bind( http::mod::Match<>( "^\\/status$" ),
        http::mod::Exec( std::bind( &cds::Server::status, _container.server, _1, _2 ) ),
        http::mod::Http() );

    _container.www->bind( http::mod::Match<>( "^\\/opds$" ),
        http::mod::Exec( std::bind( &cds::Server::opds, _container.server, _1, _2 ) ),
        http::mod::Http() );

    _container.www->bind( http::mod::Match< std::string >( "^\\/+([[:digit:]]+)$", data::KEY_KEY ),
        http::mod::Exec( std::bind( &cds::Server::node, _container.server, _1, _2 ) ),
        http::mod::Http()
    );

    _container.www->bind( http::mod::Match<std::string>( "^\\/+(.+)\\/+nodes$", data::KEY_KEY ),
        http::mod::Exec( std::bind( &cds::Server::nodes, _container.server, _1, _2 ) ),
        http::mod::Http()
    );

    _container.www->bind( http::mod::Match< std::string, std::string >( "^\\/+(ebook|movie|album|serie|artist|image)\\/+(.*)$", data::KEY_TYPE, data::KEY_NAME ),
        http::mod::Exec( std::bind( &cds::Server::keywords, _container.server, _1, _2 ) ),
        http::mod::Http()
    );

    _container.www->bind( http::mod::Match< std::string >( "^\\/+img\\/+(.*\\.jpg)$", data::KEY_KEY ),
        http::mod::Exec( [&_container](http::Request& request, http::Response& ) -> http::http_status {
            request.uri( fmt::format( "{}/{}", _container.config->tmp_directory, request.attribute( data::KEY_KEY ) ) );
            return http::http_status::OK;
        }),
        http::mod::File( "/" ),
        http::mod::Http()
    );

    _container.www->bind( http::mod::Match< std::string >( "^\\/+res\\/+([[:digit:]]+)\\.(.*)$", data::KEY_KEY, cds::PARAM_EXT ),
            http::mod::Exec( [&_container](http::Request& request, http::Response& ) -> http::http_status {
            request.uri( data::get( _container.redox, request.attribute( data::KEY_KEY ), data::KEY_PATH ) );
            return http::http_status::OK;
        }),
        http::mod::File( "/" ),
        http::mod::Http()
    );

    // register signal SIGINT and signal handler
    signal(SIGINT, signalHandler);

    while(1)
    { sleep(1); }

    return 0;
}
