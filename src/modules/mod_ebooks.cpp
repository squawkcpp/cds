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
#include "mod_ebooks.h"

#include <fstream>
#include <iostream>
#include <time.h>

#include "poppler/cpp/poppler-document.h"
#include "poppler/cpp/poppler-page.h"
#include "re2/re2.h"

#include "image.h"

#include "http/httpclient.h"

#include "../utils/amazon.h"

namespace cds {
namespace mod {

ModEbooks::ModEbooks() {}

void ModEbooks::import ( data::redis_ptr rdx, const config_ptr config ) {
    redox::Command< data::nodes_t >& c =
        rdx->commandSync< data::nodes_t > ( { REDIS_MEMBERS, data::make_key_nodes ( NodeType::ebook ) } );

    if ( c.ok() ) {
        for ( const std::string& __c : c.reply() ) {
            data::node_t _file = data::node ( rdx, __c );
            std::string _isbn = parsePdf ( _file[PARAM_PATH] );
            auto _res = utils::Amazon::bookByIsbn ( config->amazon_access_key, config->amazon_key, _isbn );
            data::node_t _book_meta;

            for ( auto& __j : _res.results ) {
                if ( _book_meta.empty() || __j[PARAM_NAME] == _file[PARAM_NAME] ) {
                    _book_meta = __j;
                }
            }

            if ( !_book_meta.empty() ) {
                //save image
                std::string _cover_path = fmt::format ( "{}/{}.jpg", config->tmp_directory, hash ( _book_meta[PARAM_COVER] ) );
                std::string _thumb_path = fmt::format ( "{}/tn_{}.jpg", config->tmp_directory, hash ( _book_meta[PARAM_COVER] ) );
                std::ofstream _ofs ( _cover_path, std::ofstream::out );
                http::get ( _book_meta[PARAM_COVER], _ofs );
                image::Image image_meta_ ( _cover_path );
                rdx->command ( {"HMSET",  make_key ( KEY_FS, hash ( _cover_path ) ),
                                PARAM_PARENT, __c,
                                PARAM_CLASS, NodeType::str ( NodeType::cover ),
                                KEY_WIDTH, std::to_string ( image_meta_.width() ),
                                KEY_HEIGHT, std::to_string ( image_meta_.height() )
                               } );
                image_meta_.scale ( 160, 160, _thumb_path );
                rdx->command ( { REDIS_ADD,
                                 make_key ( KEY_FS, hash ( _book_meta[PARAM_COVER] ), "cover" ),
                                 make_key ( KEY_FS, hash ( _cover_path ) )
                               } );
                //save ebook
                rdx->command ( {"HMSET",  make_key ( KEY_FS, __c ),
                                PARAM_NAME, _book_meta[PARAM_NAME],
                                PARAM_COMMENT, _book_meta[PARAM_COMMENT],
                                PARAM_PUBLISHER, _book_meta[PARAM_PUBLISHER],
                                PARAM_DATE, _book_meta[PARAM_DATE],
                                PARAM_ISBN, _book_meta[PARAM_ISBN],
                                PARAM_AUTHOR, _book_meta[PARAM_AUTHOR],
                                PARAM_THUMB, fmt::format ( "/img/tn_{}.jpg", hash ( _book_meta[PARAM_COVER] ) ),
                               } );
            }
        }
    }
}

const std::string ModEbooks::remove_special_characters ( const std::string& body ) {
    std::string _result = body;
    std::replace_if ( _result.begin(), _result.end(),
    [] ( char c ) {
        return c == '\n';
    }, '_' );
    _result.erase ( std::remove_if ( _result.begin(), _result.end(),
    [] ( char c ) {
        return ! ( std::isalnum ( c ) || std::isalpha ( c ) || c == '-' || c == ':' || c == ' ' || c == '_' );
    } ),
    _result.end() );
    return _result;
}

const std::string ModEbooks::find_isbn ( const std::string& body ) {
    std::string _body = body;
    static std::array< std::regex, 2 > _reg_list = std::array< std::regex, 2 > ( {
        std::regex ( "([\\d]+[- ·]?[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\dX]*)" ),
        std::regex ( "([\\d]{10,13})" )
    } );
    boost::to_upper ( _body );
    size_t _isbn_pos = _body.find ( "ISBN" );

    if (  _isbn_pos != std::string::npos ) {
        const std::string _snipped = remove_special_characters ( _body.substr ( _isbn_pos, 100 ) );

        for ( auto& __re : _reg_list ) {
            std::smatch match;

            if ( std::regex_search ( _snipped , match, __re ) && match.size() > 1 )
            { return match.str ( 1 ); }
        }
    }

    return "";
}

const std::string ModEbooks::isbn_regex = "([\\d]+[- ·]?[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\dX]*)";
const std::string ModEbooks::isbn_number_only_regex = "([\\d]{10,13})";

std::string ModEbooks::parsePdf ( const std::string& filename ) {
    poppler::document* doc = poppler::document::load_from_file  ( filename );

    if ( doc == nullptr )
    { return ""; }

    for ( size_t i = 0U; i < static_cast< size_t > ( doc->pages() ); i++ ) {
        poppler::page* p = doc->create_page ( i );
        std::string _page_string = p->text().to_latin1();
        std::string _isbn = find_isbn ( _page_string );

        if ( ! _isbn.empty() ) {
            return _isbn;
        }
    }

    return "";
}
}//namespace mod
}//namespace cds
