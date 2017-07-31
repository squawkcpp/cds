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

void ModEbooks::import( data::redox_ptr rdx, const CdsConfig& config ) {
    redox::Command<std::unordered_set< std::string > >& c =
        rdx->commandSync<std::unordered_set< std::string > >( { "SMEMBERS", "type:ebook" } );

    if( c.ok() ) {
        for( const std::string& __c : c.reply() ) {
            std::map< std::string, std::string > _file = data::node( rdx, __c );
            std::cout << "parse ebook:" << _file[PARAM_PATH] << std::endl;
            std::string _isbn = parsePdf( _file[PARAM_PATH] );
            std::cout << "parse ebook, isbn:" << _isbn << std::endl;
            auto _res = utils::Amazon::bookByIsbn( config.amazon_access_key, config.amazon_key, _isbn );
            std::cout << "Amazon Result: " << std::endl;
            std::map< std::string, std::string > _book_meta;
            for( auto& __j : _res.results ) {
                if( _book_meta.empty() || __j[PARAM_NAME] == _file[PARAM_NAME] ) {
                    _book_meta = __j;
                }
            }
            if( !_book_meta.empty() ) {
                std::cout << "parse ebook, found meta" << std::endl;

                //save image
                std::string _cover_path = fmt::format( "{}/{}.jpg", config.tmp_directory, hash( _book_meta[PARAM_COVER] ) );
                std::string _thumb_path = fmt::format( "{}/tn_{}.jpg", config.tmp_directory, hash( _book_meta[PARAM_COVER] ) );

                std::ofstream _ofs( _cover_path, std::ofstream::out );
                std::cout << "donload cover from amazon: " << _book_meta[PARAM_COVER] << std::endl;
                http::get( _book_meta[PARAM_COVER], _ofs );

                image::Image image_meta_ ( _cover_path );
                rdx->command( {"HMSET",  make_key( KEY_FS, hash( _cover_path ) ),
                               PARAM_PARENT, __c,
                               PARAM_CLASS, NodeType::name( NodeType::cover ),
                               KEY_WIDTH, std::to_string( image_meta_.width() ),
                               KEY_HEIGHT, std::to_string( image_meta_.height() )
                              } );

                image_meta_.scale ( 160, 160, _thumb_path );
                rdx->command( { "SADD",
                                make_key( KEY_FS, hash( _book_meta[PARAM_COVER] ), "cover" ),
                                make_key( KEY_FS, hash( _cover_path ) )
                              } );

                //save ebook
                rdx->command( {"HMSET",  make_key( KEY_FS, __c ),
                               PARAM_NAME, _book_meta[PARAM_NAME],
                               PARAM_COMMENT, _book_meta[PARAM_COMMENT],
                               PARAM_PUBLISHER, _book_meta[PARAM_PUBLISHER],
                               PARAM_DATE, _book_meta[PARAM_DATE],
                               PARAM_ISBN, _book_meta[PARAM_ISBN],
                               PARAM_AUTHOR, _book_meta[PARAM_AUTHOR],
                               PARAM_THUMB, fmt::format( "/cds/tn_{}.jpg", hash( _book_meta[PARAM_COVER] ) ),
                              } );
                rdx->command( { "SADD", "fs:ebook:list", __c } );
            }
        }
    }
}

const std::string ModEbooks::remove_special_characters( const std::string& body ) {
    std::string _result = body;
    std::replace_if( _result.begin(), _result.end(),
    [](char c) {
        return c == '\n';
    }, '_' );
    _result.erase( std::remove_if( _result.begin(), _result.end(),
    [](char c) {
        return !( std::isalnum( c ) || std::isalpha(c ) || c == '-' || c == ':' || c == ' ' || c == '_' );
    } ),
    _result.end());
    return _result;
}

const std::string ModEbooks::find_isbn ( const std::string& body ) {
    std::string _body = body;
    static std::array< std::regex, 2 > _reg_list = std::array< std::regex, 2 > ({
        std::regex( "([\\d]+[- ·]?[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\dX]*)" ),
        std::regex( "([\\d]{10,13})" )
    });

    boost::to_upper( _body );
    size_t _isbn_pos = _body.find( "ISBN" );
    if(  _isbn_pos != std::string::npos ) {
        const std::string _snipped = remove_special_characters( _body.substr( _isbn_pos, 100 ) );
        std::cout << "\nsearch for ISBN in subsctring \"" << _snipped << "\"" << std::endl; //TODO
        for( auto& __re : _reg_list ) {
            std::smatch match;
            if (std::regex_search( _snipped , match, __re ) && match.size() > 1)
            {
                return match.str( 1 );
            }
        }
    }
    return "";
}

const std::string ModEbooks::isbn_regex = "([\\d]+[- ·]?[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\d]*[- ·]{1}[\\dX]*)";
const std::string ModEbooks::isbn_number_only_regex = "([\\d]{10,13})";

std::string ModEbooks::parsePdf( const std::string & filename ) {

    std::cout << "search ISBN in: " << filename << std::endl;
    poppler::document * doc = poppler::document::load_from_file  ( filename );
    if( doc == nullptr ) {
        std::cout << "unable to open pdf document" << std::endl;
        return "";
    }

    for( size_t i=0U; i<static_cast< size_t >( doc->pages() ); i++ ) {
        poppler::page * p = doc->create_page( i );
        std::string _page_string = p->text().to_latin1();
        std::string _isbn = find_isbn( _page_string );
        if( ! _isbn.empty() ) {
            std::cout << "found isbn: " << _isbn << std::endl;
            return _isbn;
        }
    }
    std::cout << "\nno ISBN found in: " << filename << std::endl;
    return "";
}
}//namespace mod
}//namespace cds
