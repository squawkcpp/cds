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

#include "rapidxml_ns.hpp"
#include "rapidxml_ns_print.hpp"
#include "rapidxml_ns_utils.hpp"

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

#include "_utils.h"
#include "config.h"
#include "datastore.h"
#include "scanner.h"

namespace cds {

Server::Server ( const std::string& redis, /** @param redis redis host */
                 const short port /** @param port the redis port. */ )
    : redis_ ( data::make_connection ( redis, port ) ), config_ ( json ( data::config ( redis_ ) ) ) {
    if ( !sub_.connect( redis, port ) ) {
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

    for( auto& __type : _internal::menu ) {
        writer.String( __type.at( key::TYPE ).c_str() );
        writer.Int( data::children_count( redis_, __type.at( key::TYPE ) ) );
    }

    writer.EndObject();
    writer.String ( "types" );
    writer.StartObject();
    data::command_t _command = { redis::SMEMBERS, key::MIME };
    redox::Command< data::nodes_t >& c =
        redis_->commandSync< data::nodes_t >( _command );

    if ( c.ok() ) {
        for ( const std::string& __mime : c.reply() ) {
            writer.String ( __mime.c_str() );
            auto _res = redis_->get ( data::make_key( key::MIME, __mime ) );
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
    SPDLOG_DEBUG(spdlog::get ( LOGGER ), "HTTP>/node (key={})", request.attribute ( key::KEY ) );
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();
    writer.String ( key::KEY.c_str() );
    writer.String ( request.attribute ( key::KEY ).c_str() );

    auto& _item = redis_->commandSync<data::command_t>({
        redis::HGETALL, data::make_key_node ( request.attribute ( key::KEY ) )
    });
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

http::http_status Server::path ( http::Request& request, http::Response& response ) {
    std::string _key = request.attribute ( key::KEY );
    std::vector< std::map< std::string, std::string > > values;

    const std::string _name = data::get( redis_, _key, param::NAME );
    values.insert( values.begin(), { {"key", _key}, {"name", _name} } );
    while( _key != param::ROOT ) {
        const std::string _parent = data::get( redis_, _key, param::PARENT );
        const std::string _name = data::get( redis_, _parent, param::NAME );
        values.insert( values.begin(), { {"key", _parent}, {"name", _name} } );
        _key = _parent;
    }

    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );

    writer.StartArray();
    for( auto& __c : values ) {
        writer.StartObject();
        writer.String ( "key" );
        writer.String ( __c["key"].c_str() );
        writer.String ( "name" );
        writer.String ( __c["name"].c_str() );
        writer.EndObject();
    }
    writer.EndArray();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::sort ( http::Request& request, http::Response& response ) {
    const std::string _key = request.attribute ( key::KEY );

    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );

    writer.StartArray();
    redox::Command<data::command_t>& _c = redis_->commandSync< data::command_t >(
        { redis::SMEMBERS, data::make_key( "fs", _key, "list", "sort", "list" ) } );
    if( _c.ok() ) {
        for( const std::string& __c : _c.reply() ) {
            writer.String ( __c.c_str() );
        }
    }
    writer.EndArray();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::nodes ( http::Request& request, http::Response& response ) {

    std::string _key = request.attribute ( key::KEY );
    int _index = ( request.contains_attribute( "index" ) &&
                   ( request.attribute( "index" ).find_first_not_of( "0123456789" ) == std::string::npos) ?
                       std::stoi( request.attribute( "index" ) ) : 0 );
    int _count = ( request.contains_attribute( "count" ) &&
                   ( request.attribute( "count" ).find_first_not_of( "0123456789" ) == std::string::npos) &&
                   std::stoi( request.attribute( "count" ) ) < 8192 ?
                       std::stoi( request.attribute( "count" ) ) : -1 );
    std::string _sort = ( request.contains_attribute( "sort" ) && !request.attribute( "sort" ).empty() ?
                             request.attribute( "sort" ) : "default" );
    std::string _order = ( request.contains_attribute( "order" ) && ( request.attribute( "order" )=="asc" || request.attribute( "order" )=="desc" ) ?
                              request.attribute( "order" ) : "asc" );
    std::string _filter = ( request.contains_attribute( "filter" ) ? request.attribute( "filter" ) : "" );

    SPDLOG_DEBUG(spdlog::get ( LOGGER ), "HTTP>/nodes (key={}, index={}, count={}, sort={}, order={}, filter={})", _key, _index, _count, _sort, _order, _filter );

    int _result = 0;
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();

    writer.String ( "nodes" );
    writer.StartArray();

    data::children( redis_, _key, _index, _count, _sort, _order, _filter, [this,&writer,&_result]( const std::string& key ) {
        ++_result;
        writer.StartObject();
        writer.String ( key::KEY.c_str() );
        writer.String ( key.c_str() );

        auto n = data::node( redis_, key );
        for ( auto& __item : n ) {
            writer.String ( __item.first.c_str() );
            writer.String ( __item.second.c_str() );
        }
        writer.EndObject();
    });
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

http::http_status Server::files ( http::Request& request, http::Response& response ) {

    data::NodeType::Enum _class = data::NodeType::parse( request.attribute ( param::CLASS ) );
    std::string _parent = request.attribute ( key::KEY );
    int _index = ( request.contains_attribute( "index" ) &&
                   ( request.attribute( "index" ).find_first_not_of( "0123456789" ) == std::string::npos) ?
                       std::stoi( request.attribute( "index" ) ) : 0 );
    int _count = ( request.contains_attribute( "count" ) &&
                   ( request.attribute( "count" ).find_first_not_of( "0123456789" ) == std::string::npos) &&
                   std::stoi( request.attribute( "count" ) ) < 8192 ?
                       std::stoi( request.attribute( "count" ) ) : -1 );

    SPDLOG_DEBUG(spdlog::get ( LOGGER ), "HTTP>/files (key={}, class={})", _parent, data::NodeType::str( _class ) );

    int _result = 0;
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartObject();

    writer.String ( "nodes" );
    writer.StartArray();

    data::files( redis_, _parent, _class, _index, _count, [this,&writer,&_result]( const std::string& key ) {
        ++_result;
        writer.StartObject();
        writer.String ( key::KEY.c_str() );
        writer.String ( key.c_str() );

        auto n = data::node( redis_, key );
        for ( auto& __item : n ) {
            writer.String ( __item.first.c_str() );
            writer.String ( __item.second.c_str() );
        }
        writer.EndObject();
    });
    writer.EndArray();

    //add counters
    writer.String( "count" );
    writer.Int( data::files_count( redis_, _parent, _class ) );
    writer.String( "result" );
    writer.Int( _result );

    writer.EndObject();
    response << sb.GetString();
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::JSON ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::opds( http::Request& request, http::Response& response ) {
    std::cout << request << std::endl;

    rapidxml_ns::xml_document<> doc_;
    auto  root_node_ = element<rapidxml_ns::xml_node<>>( &doc_, &doc_, "feed", "" );
    attr( &doc_, root_node_, "xmlns", "http://www.w3.org/2005/Atom" );
    attr( &doc_, root_node_, "xmlns:dc", "http://purl.org/dc/terms/" );
    attr( &doc_, root_node_, "xmlns:opds", "http://opds-spec.org/2010/catalog" );

    element<rapidxml_ns::xml_node<>>( &doc_, root_node_, "id", "uuid:433a5d6a-0b8c-4933-af65-4ca4f02763eb" ); //TODO create uuid
    element<rapidxml_ns::xml_node<>>( &doc_, root_node_, "title", "Squawk Bookshelf" ); //TODO create uuid

    data::children( redis_, param::EBOOK, 0, -1, "", "", "", [this,&doc_,&root_node_](const std::string& key ) {
        auto n = data::node( redis_, key );
        rapidxml_ns::xml_node<>*  entry_node_ = element<rapidxml_ns::xml_node<>>( &doc_, root_node_, "entry", "" );

        element<rapidxml_ns::xml_node<>>( &doc_, entry_node_, "title", n.at( "name" ) );
//            <id>urn:uuid:6409a00b-7bf2-405e-826c-3fdff0fd0734</id>
//            <updated>2010-01-10T10:01:11Z</updated>
        if( n.find( "author" ) != n.end() ) {
            auto _author_n = element<rapidxml_ns::xml_node<>>( &doc_, entry_node_, "author", "" );
            element<rapidxml_ns::xml_node<>>( &doc_, _author_n, "name", n.at( "author" ) );
        }
//            <dc:language>en</dc:language>
//            <dc:issued>1917</dc:issued>
//            <category scheme="http://www.bisg.org/standards/bisac_subject/index.html"
//                      term="FIC020000"
//                      label="FICTION / Men's Adventure"/>

        if( n.find( "comment" ) != n.end() )
        { element<rapidxml_ns::xml_node<>>( &doc_, entry_node_, "summary", n.at( "comment" ) ); }

        if( n.find( "thumb" ) != n.end() ) {
            auto _image_n = element<rapidxml_ns::xml_node<>>( &doc_, entry_node_, "link", "" );
            attr( &doc_, _image_n, "rel", "http://opds-spec.org/image" );
            attr( &doc_, _image_n, "href", n.at( "thumb" ) );
            attr( &doc_, _image_n, "type", "image/jpeg" );
        }
//            <link rel="http://opds-spec.org/image/thumbnail"
//                  href="/covers/4561.thmb.gif"
//                  type="image/gif"/>

        auto _res_n = element<rapidxml_ns::xml_node<>>( &doc_, entry_node_, "link", "" );
        attr( &doc_, _res_n, "rel", "http://opds-spec.org/acquisition" );
        attr( &doc_, _res_n, "href", fmt::format( "/res/{}{}", /*config_->listen_address, config_->http_port,*/ key, n.at( "ext" ) ) );
        attr( &doc_, _res_n, "type", n.at( "mimeType" ) );
    });

    response << doc_;
    response.parameter ( http::header::CONTENT_TYPE, http::mime::mime_type ( http::mime::XML ) );
    response.parameter ( "Access-Control-Allow-Origin", "*" );
    return http::http_status::OK;
}

http::http_status Server::sug ( http::Request& request, http::Response& response ) {
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartArray();

    auto& _c = redis_->commandSync< std::set< std::string > > (
        { "FT.SUGGET", "autocomplete", request.attribute( param::NAME ) } );

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

http::http_status Server::keywords ( http::Request& request, http::Response& response ) {
    std::cout << "keywords " << request.attribute( key::TYPE ) << "=" << request.attribute ( param::NAME ) << std::endl;
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer ( sb );
    writer.StartArray();

    auto& _c = redis_->commandSync< std::set< std::string > > (
        { redis::ZRANGE, data::make_key ( key::FS, key::TAG, request.attribute ( param::NAME ) ), "0", "-1" } );

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
        { Scanner::flush( redis_ ); }
        //start import
        cds::Scanner::import_files ( redis_, config_ );

    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception in rescan." );
        _errc = std::error_code ( 1, std::generic_category() );
    }
    fn( _errc );
}
}//namespace cds
