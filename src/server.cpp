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
#include "server.h"

#include <map>
#include <string>
#include <vector>

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

#include "config.h"
#include "datastore.h"
#include "scanner.h"

namespace cds {

Server::Server ( const std::string& redis, /** @param redis redis host */
                 const short port /** @param port the redis port. */ )
    : redis_ ( cds::data::make_connection ( redis, port ) ), config_ ( json ( data::config ( redis_ ) ) ) {
    if ( !sub_.connect() ) {
        spdlog::get ( LOGGER )->error ( "can not subscribe to redis queue" );
    } else {
        sub_.subscribe ( EVENT_SCANNER, [] ( const std::string & topic, const std::string & msg ) {
            spdlog::get ( LOGGER )->debug ( "EVENT:{}:{}", topic, msg );
        } );
        sub_.subscribe ( EVENT_RESCAN, [this] ( const std::string & topic, const std::string & msg ) {
            spdlog::get ( LOGGER )->debug ( "COMMAND:{}:{}", topic, msg );
            if( !rescanning_ ) {
                rescanning_ = true;
                redis_->publish ( EVENT_SCANNER, EVENT_START );
                scanner_thread_ = std::make_unique< std::thread >( &Server::rescan_, this, msg == "true", [this]( std::error_code& errc ) {
                    spdlog::get( LOGGER )->debug( "scanner fisnished: {}", errc.message() );
                    redis_->publish ( EVENT_SCANNER, EVENT_END );
                    rescanning_ = false;
            });
            scanner_thread_->detach();
            } else spdlog::get( LOGGER )->debug( "scanner already running." );
        } );
    }
}

http::http_status Server::config ( http::Request&, http::Response& response ) {
    response << data::config ( redis_ );
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::rescan ( http::Request& request, http::Response& response ) {
    if( !rescanning_ ) {
        redis_->publish ( EVENT_RESCAN, ( request.contains_attribute ( "flush" ) && request.attribute ( "flush" ) == "true" ) ? "true" : "false" );
        response << "{\"code\":200, \"result\":\"OK\"}";
    } else {
        response << "{\"code\":400, \"result\":\"ressource busy.\"}";
    }
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::ACCEPTED;
}

http::http_status Server::status ( http::Request&, http::Response& response ) {
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();
    writer.String ( "version" );
    writer.String ( VERSION.c_str() );
    writer.String ( "rescan" );
    writer.String ( ( rescanning_ ? "true" : "false" ) );

    writer.String ( "content" );
    writer.StartObject();

    for ( auto& __type : NodeTypes ) {
        writer.String ( str ( __type ).c_str() );
        writer.Int ( data::count ( redis_, data::make_key_nodes( __type ) ) );
    }

    writer.EndObject();
    writer.String ( "types" );
    writer.StartObject();
    std::vector< std::string > _command = { REDIS_MEMBERS, data::make_key_node( KEY_MIME ) };
    redox::Command< data::nodes_t >& c =
        redis_->commandSync< data::nodes_t >( _command );

    if ( c.ok() ) {
        for ( const std::string& __mime : c.reply() ) {
            writer.String ( __mime.c_str() );
            auto _res = redis_->get ( make_key ( KEY_FS, KEY_MIME, __mime ) );
            writer.String ( _res.c_str() );
        }
    }

    writer.EndObject();
    writer.EndObject();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::node ( http::Request& request, http::Response& response ) {
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();
    auto& _item = redis_->commandSync<std::vector< std::string > > (
    { REDIS_HGETALL, make_key ( KEY_FS, request.attribute ( PARAM_KEY ) ) } );

    if ( _item.ok() ) {
        for ( auto& __i : _item.reply() ) {
            writer.String ( __i.c_str() );
        }
    }

    writer.EndObject();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::nodes ( http::Request& request, http::Response& response ) {

    std::string _key = request.attribute ( PARAM_KEY );
    if( _key.empty() || _key == "root" )
    { _key = "/"; }

    int _result = 0;
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();

    writer.String ( "nodes" );
    writer.StartArray();

    data::children( redis_, _key, [this,&writer,&_result]( const std::string& key ) {
        ++_result;
        writer.StartObject();
        writer.String ( PARAM_KEY.c_str() );
        writer.String ( key.c_str() );

        auto n = data::node( redis_, key );
        for ( auto& __item : n ) {
            writer.String ( __item.first.c_str() );
            writer.String ( __item.second.c_str() );
        }
        writer.EndObject();
    });

//    std::vector< std::string > _command;
//    _command.push_back ( REDIS_SUNION );
//    data::FOR_NODE ( redis_, _command, [&writer] ( const std::string & key, data::node_t n ) {
//        writer.StartObject();
//        writer.String ( PARAM_KEY.c_str() );
//        writer.String ( key.c_str() );

//        for ( auto& __item : n ) {
//            writer.String ( __item.first.c_str() );
//            writer.String ( __item.second.c_str() );
//        }

//        writer.EndObject();
//    } );

    writer.EndArray();

    //add counters
    writer.String( "count" );
    writer.Int( data::children_count( redis_, _key ) );
    writer.String( "result" );
    writer.Int( _result );

    writer.EndObject();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::mod ( http::Request& request, http::Response& response ) {

    int _result = 0;
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();
    writer.String ( "nodes" );
    writer.StartArray();

    data::nodes( redis_, NodeType::parse( mod_key ( request.attribute ( PARAM_KEY ) ) ), [this,&writer,&_result]( const std::string& key ) {
        _result++;
        data::node_t n = data::node( redis_, key );
        writer.StartObject();
        writer.String ( PARAM_KEY.c_str() );
        writer.String ( key.c_str() );

        for ( auto& __item : n ) {
            writer.String ( __item.first.c_str() );
            writer.String ( __item.second.c_str() );
        }

        writer.EndObject();
    });
    writer.EndArray();

    writer.String( "count" );
    writer.Int( data::nodes_count( redis_, NodeType::parse( mod_key ( request.attribute ( PARAM_KEY ) ) ) ) );
    writer.String( "result" );
    writer.Int( _result );

    writer.EndObject();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::keywords ( http::Request& request, http::Response& response ) {
    std::cout << "keywords " << request.attribute( PARAM_TYPE ) << "=" << request.attribute ( PARAM_NAME ) << std::endl;
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartArray();

    std::vector< std::string > _command = {
        REDIS_ZRANGE, make_key ( KEY_FS, request.attribute( PARAM_TYPE ), "keyword", request.attribute ( PARAM_NAME ) ), "0", "-1"
    };
    redox::Command< std::set< std::string > >& _c = redis_->commandSync< std::set< std::string > > ( _command );

    if ( _c.ok() ) {
        for ( const std::string& __c : _c.reply() ) {
            writer.String ( __c.c_str() );
        }
    }

    writer.EndArray();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

void Server::rescan_ ( bool flush, std::function<void( std::error_code& )> fn ) {
    std::error_code _errc;
    try {
        //flush database
        if ( flush )
        { data::flush( redis_, KEY_FS ); }
        //start import
        cds::Scanner::import_files ( redis_, config_ );

    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception in rescan." );
        _errc = std::error_code ( 1, std::generic_category() );
    }
    fn( _errc );
}
}//namespace cds
