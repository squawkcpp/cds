///*
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//*/
//#include "mod_ebooks.h"

//#include <fstream>
//#include <iostream>
//#include <time.h>

//#include <boost/filesystem.hpp>

//#include "poppler/cpp/poppler-document.h"
//#include "poppler/cpp/poppler-page.h"
//#include "re2/re2.h"

//#include "http/httpclient.h"

//#include "../_utils.h"
//#include "../datastore.h"
//#include "../utils/amazon.h"
//#include "../utils/image.h"

//namespace cds {
//namespace mod {

//ModEbooks::ModEbooks() {}

//void ModEbooks::import ( data::redis_ptr redis, const config_ptr config, const std::string& key ) {
//    try {
//        data::node_t _file = data::node ( redis, key );
//        std::string _isbn = parsePdf ( _file[param::PATH] );

//        // we have to wait some time, otherwise amazon blocks the requests.
//        sleep ( SLEEP );
//        //TODO auto _res = utils::Amazon::bookByIsbn ( config->amazon_access_key, config->amazon_key, _isbn );
//        data::node_t _book_meta;

////TODO        for ( auto& __j : _res.results ) {
////            if ( _book_meta.empty() || __j[param::NAME] == _file[param::NAME] ) {
////                _book_meta = __j;
////            }
////        }

//        if ( !_book_meta.empty() ) {
//            //save image
//            std::string _cover_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, data::hash ( _book_meta[param::COVER] ) );
//            std::ofstream _ofs ( _cover_path, std::ofstream::out );
//            http::get ( _book_meta[param::COVER], _ofs );

//            utils::Image image_meta_ ( _cover_path );
//            image_meta_.scale ( config->tmp_directory, ECoverSizes::MED, data::hash ( _book_meta[param::COVER] ) );
//            image_meta_.scale ( config->tmp_directory, ECoverSizes::TN, data::hash ( _book_meta[param::COVER] ) );


//            data::save ( redis, data::hash ( _cover_path ), {
//                { param::PARENT, key },
//                { param::CLASS, data::NodeType::str ( data::NodeType::cover ) },
//                { param::WIDTH, std::to_string ( image_meta_.width() ) },
//                { param::HEIGHT, std::to_string ( image_meta_.height() ) }
//            });
//            data::add_nodes( redis, data::hash ( _book_meta[param::COVER] ), data::NodeType::cover,
//                             data::make_key_node ( data::hash ( _cover_path ) ), data::time_millis() );

//            //save ebook
//            data::save ( redis, key, {
//                { param::NAME, _book_meta[param::NAME] },
//                { param::COMMENT, _book_meta[param::COMMENT] },
//                { param::PUBLISHER, _book_meta[param::PUBLISHER] },
//                { param::DATE, _book_meta[param::DATE] },
//                { param::ISBN, _book_meta[param::ISBN] },
//                { param::AUTHOR, _book_meta[param::AUTHOR] },
//                { param::THUMB, utils::make_cover_uri ( ECoverSizes::TN, data::hash ( _book_meta[param::COVER] ) ) },
//            });
//        }
//        auto _last_write_time = boost::filesystem::last_write_time( data::get( redis, key, param::PATH ) );
//        data::add_nodes( redis, data::NodeType::ebook, key, static_cast< unsigned long>( _last_write_time ) );
//    } catch ( ... ) {
//        spdlog::get ( LOGGER )->error ( "exception mod ebooks." );
//    }
//}

//const std::string ModEbooks::remove_special_characters ( const std::string& body ) {
//    std::string _result = body;
//    std::replace_if ( _result.begin(), _result.end(),
//    [] ( char c ) {
//        return c == '\n';
//    }, '_' );
//    _result.erase ( std::remove_if ( _result.begin(), _result.end(),
//    [] ( char c ) {
//        return ! ( std::isalnum ( c ) || std::isalpha ( c ) || c == '-' || c == ':' || c == ' ' || c == '_' );
//    } ),
//    _result.end() );
//    return _result;
//}

//const std::string ModEbooks::find_isbn ( const std::string& body ) {
//    std::string _body = body;
//    static std::array< std::regex, 2 > _reg_list = std::array< std::regex, 2 > ( {
//        { std::regex ( "([\\d]+[- ·]?[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\dX]*)" ),
//          std::regex ( "([\\d]{10,13})" ) }
//    } );
//    boost::to_upper ( _body );
//    size_t _isbn_pos = _body.find ( "ISBN" );

//    if (  _isbn_pos != std::string::npos ) {
//        const std::string _snipped = remove_special_characters ( _body.substr ( _isbn_pos, 100 ) );

//        for ( auto& __re : _reg_list ) {
//            std::smatch match;

//            if ( std::regex_search ( _snipped , match, __re ) && match.size() > 1 )
//            { return match.str ( 1 ); }
//        }
//    }

//    return "";
//}

//const std::string ModEbooks::isbn_regex = "([\\d]+[- ·]?[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\dX]*)";
//const std::string ModEbooks::isbn_number_only_regex = "([\\d]{10,13})";

//std::string ModEbooks::parsePdf ( const std::string& filename ) {
//    poppler::document* doc = poppler::document::load_from_file  ( filename );

//    if ( doc == nullptr )
//    { return ""; }

//    for ( size_t i = 0U; i < static_cast< size_t > ( doc->pages() ); i++ ) {
//        poppler::page* p = doc->create_page ( static_cast< int >( i ) );
//        std::string _page_string = p->text().to_latin1();
//        std::string _isbn = find_isbn ( _page_string );

//        if ( ! _isbn.empty() ) {
//            return _isbn;
//        }
//    }

//    return "";
//}
//}//namespace mod
//}//namespace cds
