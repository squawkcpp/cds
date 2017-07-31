#include "server.h"

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "fmt/format.h"
#include "spdlog/spdlog.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include "http/mod/exec.h"
#include "http/mod/file.h"
#include "http/mod/match.h"
#include "http/mod/method.h"

#include "datastore.h"
#include "scanner.h"

namespace cds {

Server::Server( std::shared_ptr< http::Server< http::HttpServer > > web_server,
                const std::string& redis, const short port,
                const std::string& tmp_directory, const std::string& tmdb_key,
                const std::string& amazon_access_key, const std::string& amazon_key,
                const std::vector< std::string >& media ) : redis_( data::make_connection( redis, port ) ), redis_ip_(redis), redis_port_(port) {

    spdlog::set_level(spdlog::level::trace);
    auto console = spdlog::stdout_color_mt(LOGGER);

    if( data::config_exists( redis_ ) ) {
        config_ = json( data::config( redis_ ) );
    }
    if( !media.empty() )
    { config_.media = media; }
    if( !tmp_directory.empty() )
    { config_.tmp_directory = tmp_directory; }
    if( !tmdb_key.empty() )
    { config_.tmdb_key = tmdb_key; }
    if( !amazon_access_key.empty() )
    { config_.amazon_access_key = amazon_access_key; }
    if( !amazon_key.empty() )
    { config_.amazon_key = amazon_key; }

    //validate & store configuration
    //TODO validate config
    data::config( redis_, json( config_ ) );

    web_server->bind( http::mod::Match<>( "^/cds/config$" ),
        http::mod::Exec( [this](http::Request&, http::Response& response ) -> http::http_status {
            response << data::config( redis_ );
            response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
            return http::http_status::OK;
        }),
        http::mod::Http() );

    web_server->bind( http::mod::Match<>( "^/rescan$" ),
        http::mod::Exec( [this](http::Request& request, http::Response& response ) -> http::http_status {
            redis_->publish( EVENT_RESCAN, ( request.contains_attribute( "flush" ) && request.attribute( "flush" ) == "true" ) ? "true" : "false" );
            response << "{\"code\":200, \"result\":\"OK\"}";
            response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
            return http::http_status::OK;
        } ),
        http::mod::Http() );

    web_server->bind( http::mod::Match<>( "^/cds/status$" ),
        http::mod::Exec( [this](http::Request&, http::Response& response ) -> http::http_status {

            using namespace rapidjson;
            StringBuffer sb;
            PrettyWriter<StringBuffer> writer(sb);
            writer.StartObject();

            auto _status = data::status( redis_ );
            for( auto& __s : _status ) {
                writer.String( __s.first.c_str() );
                writer.String( __s.second.c_str() );
            }
            writer.EndObject();
            response << sb.GetString();
            response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
            return http::http_status::OK;
        }),
        http::mod::Http() );

    web_server->bind( http::mod::Match<>( "^\\/cds\\/?$" ),
        http::mod::Exec( [this](http::Request&, http::Response& response ) -> http::http_status {
        data::load_item( response, redis_, "/" );
        response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
        return http::http_status::OK;
    }),
    http::mod::Http() );

    web_server->bind( http::mod::Match<>( "^\\/+cds\\/+nodes\\/?$" ),
        http::mod::Exec( [this](http::Request&, http::Response& response ) -> http::http_status {
        data::types_t _types;
        data::load_nodes( response, redis_, "/", _types );
        response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
        return http::http_status::OK;
    }),
    http::mod::Http() );

    web_server->bind( http::mod::Match< std::string >( "^\\/+cds\\/+(album|artist|ebook|serie|movie|fs|photo)\\/+nodes$", PARAM_KEY ),
                      http::mod::Exec( [this](http::Request& request, http::Response& response ) -> http::http_status {

        std::cout << "load module root:" << request.attribute( PARAM_KEY ) << std::endl;

        data::types_t _types = ( request.contains_attribute( PARAM_TYPES ) && !request.attribute( PARAM_TYPES ).empty() ?
                          data::split_types( request.attribute( PARAM_TYPES ) ) : data::types_t() );
        data::load_nodes( response, redis_, request.attribute(PARAM_KEY), _types );
        response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
        return http::http_status::OK;
    }),
    http::mod::Http() );

    //web_server->bind( http::mod::Match< std::string >( "^\\/+cds\\/+(album|artist|ebook|serie|movie|fs|photo|[[:digit:]]+)\\/+nodes$", PARAM_KEY ),
    web_server->bind( http::mod::Match< std::string >( "^\\/+cds\\/+(.*)\\/+nodes$", PARAM_KEY ),
                      http::mod::Exec( [this](http::Request& request, http::Response& response ) -> http::http_status {

        data::types_t _types = ( request.contains_attribute("types") && !request.attribute( "types" ).empty() ?
                          data::split_types( request.attribute( "types" ) ) : data::types_t() );
        data::load_nodes( response, redis_, request.attribute(PARAM_KEY), _types );
        response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
        return http::http_status::OK;
    }),
    http::mod::Http() );

    web_server->bind( http::mod::Match< std::string >( "^\\/+cds\\/+([[:digit:]]+)$", PARAM_KEY ),
        http::mod::Exec( [this](http::Request& request, http::Response& response ) -> http::http_status {
            data::load_item( response, redis_, request.attribute(PARAM_KEY) );
            response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type( http::mime::JSON ) );
            return http::http_status::OK;
        }),
        http::mod::Http()
    );

    web_server->bind( http::mod::Match< std::string >( "^\\/+cds\\/+(.*\\..*)$", PARAM_KEY ),
        http::mod::Exec( [this](http::Request& request, http::Response& ) -> http::http_status {
            request.uri( fmt::format( "{}/{}", config_.tmp_directory, request.attribute( PARAM_KEY ) ) );
            return http::http_status::OK;
        }),
        http::mod::File( "/" ),
        http::mod::Http()
    );
    web_server->bind( http::mod::Match< std::string >( "^\\/+res\\/([[:digit:]]+)\\.(.*)$", PARAM_KEY, PARAM_EXT ),
        http::mod::Exec( [this](http::Request& request, http::Response& ) -> http::http_status {
            request.uri( data::path( redis_, request.attribute( PARAM_KEY ) ) );
            return http::http_status::OK;
        }),
        http::mod::File( "/" ),
        http::mod::Http()
    );

    if( !sub_.connect() ) {
        console->error( "can not subscribe to redis queue" );
    } else {
        sub_.subscribe(EVENT_SCANNER, [console](const std::string& topic, const std::string& msg) {
          console->debug( "EVENT:{}:{}", topic, msg );
        });
        sub_.subscribe( EVENT_RESCAN, [console,this](const std::string& topic, const std::string& msg) {
          console->debug( "COMMAND:{}:{}", topic, msg );
          rescan( config_, ( msg == "true" ) );
        });
    }
}
void Server::rescan ( CdsConfig& c, bool flush ) {
    auto console = spdlog::get(LOGGER);
    if( rescan_mutex_.try_lock() ) {
        //TODO redis_->publish( EVENT_SCANNER, "start" );
        data::make_connection( redis_ip_, redis_port_ )->publish( EVENT_SCANNER, "start" );
        try {
            //load the files
            if( flush ) {
                //flush database
                redis_->command( {"FLUSHALL"} );
                //store configuration
                data::config( redis_, json( c ) );
            }
            //start import
            cds::Scanner _scanner;
            _scanner.import_files( redis_, c );

        } catch( ... ) {
            console->error( "exception in rescan." );
        }
        data::make_connection( redis_ip_, redis_port_ )->publish( EVENT_SCANNER, "end" );
        rescan_mutex_.unlock();
    }
}
}//namespace cds
