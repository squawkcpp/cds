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
#ifndef _CDS_UTILS_H
#define _CDS_UTILS_H

#include <algorithm>
#include <regex>
#include <map>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include "spdlog/spdlog.h"

#include <boost/algorithm/string.hpp>

#include "rapidxml_ns.hpp"
#include "rapidxml_ns_print.hpp"

#include "fmt/format.h"

namespace cds {

#ifdef DEBUG
    static const bool CDS_DEBUG = true;
#else
static const bool CDS_DEBUG = false;
#endif

static const std::string LOGGER = "cds";
static const std::string VERSION = CDS_VERSION;

static const int SLEEP = 10;

static const std::string EVENT_RESCAN = "cds:rescan";

static const char* PARAM_TMP_DIRECTORY      = "tmp-directory";
static const char* PARAM_TMDB_KEY           = "tmdb-key";
static const char* PARAM_AMAZON_KEY         = "amazon-key";
static const char* PARAM_AMAZON_ACCESS_KEY  = "amazon-access-key";
static const char* PARAM_REDIS              = "redis";
static const char* PARAM_REDIS_PORT         = "redis-port";
static const char* PARAM_LISTEN_ADDRESS     = "listen";
static const char* PARAM_HTTP_PORT          = "http-port";
static const char* PARAM_DIRECTORY          = "directory";

static std::vector< std::string > trash_words ( {
    "flac", "24bit", "720p", "1080p", "amzn", "webrip", "dd5.1", "x264", "mkv",
    "hdtv", "web-dl", "h264", "bdrip", "dts", "bluray", "-hdc", "mp4", "-2hd",
    "-rarbg", "[rarbg]", "-fgt", "-japhson", "-fgt", "-ntb", "-moritz", "-avs",
    "-dumbension", "-it00nz", "-cravers", "-rovers", "-sparks", "[96-24]", "audio fidelity",
                                                    "180g lp", "24-96", "-hdma5", "-chd"
});

static std::vector< std::string > skip_words (
{ "the ", "der ", "die ", "das " } );

static std::array< std::string, 3 > album_cover_names (
{ {"cover", "front", "folder"} } );

static std::array< std::string, 6 > metadata_relations (
{ {"allmusic", "IMDb", "official homepage", "image", "wikipedia", "discogs"} } );

struct ECoverSizes {
    enum Enum { TN, MED };
    static std::string str ( const Enum& size ) {
        switch ( size ) {
            case TN: return "tn";
            case MED: return "med";
        }
    }
    static double size ( const Enum& size ) {
        switch ( size ) {
            case TN: return IMAGE_TN_SIZE;
            case MED: return IMAGE_MED_SIZE;
        }
    }
private:
    static constexpr double IMAGE_MED_SIZE = 768;
    static constexpr double IMAGE_TN_SIZE = 160;
};

inline std::string remove_skip_list ( const std::vector< std::string >& words, const std::string& s ) {
    std::string _return_value = s;

    for ( auto& __w : words ) {
        size_t _pos = 0;
        std::string _lower = boost::algorithm::to_lower_copy ( _return_value );

        while ( ( _pos = _lower.find ( __w, _pos ) ) != std::string::npos ) {
            _return_value.erase ( _pos, __w.size() );
            _lower.erase ( _pos, __w.size() );
        }
    }

    std::regex brace_re ( "(\\( *\\))" );
    _return_value = std::regex_replace ( _return_value, brace_re, "" );
    std::regex brackets_re ( "(\\[ *\\])" );
    _return_value = std::regex_replace ( _return_value, brackets_re, "" );
    std::regex underscore_re ( "([_*])" );
    _return_value = std::regex_replace ( _return_value, underscore_re, " " );
    std::regex dot_re ( "([\\.*])" );
    _return_value = std::regex_replace ( _return_value, dot_re, " " );
    boost::algorithm::trim ( _return_value );
    return _return_value;
}
inline std::string clean_string ( const std::string& s ) {
    std::string _result = boost::algorithm::to_lower_copy ( s );

    for ( auto& _word : skip_words ) {
        size_t _position;

        while ( ( _position = _result.find ( _word ) ) != std::string::npos ) {
            _result.erase ( _position, _word.size() );
        }
    }

    boost::algorithm::trim ( _result );
    return _result;
}
inline std::string path ( const std::string& filename ) {
    std::string _path;

    if ( filename.find ( "/" ) != std::string::npos ) {
        _path = filename.substr ( 0, filename.rfind ( "/" ) );
    } else {
        _path = filename;
    }

    return _path;
}
inline std::string filename ( const std::string& filename, bool extension = true ) {
    std::string _filename;

    if ( filename.find ( "/" ) != std::string::npos ) {
        _filename = filename.substr ( filename.rfind ( "/" ) + 1 );
    } else {
        _filename = filename;
    }

    if ( !extension ) {
        if ( _filename.find ( "." ) != std::string::npos ) {
            _filename.erase ( _filename.rfind ( "." ) );
        }
    }

    return _filename;
}
inline std::string remove_extension ( const std::string& file ) {
    if ( file.find ( "." ) != std::string::npos ) {
        return file.substr ( 0, file.rfind ( "." ) );
    } else {
        return std::string ( file );
    }
}

inline std::string clean_isbn ( std::string isbn ) {
    if ( isbn.find ( "urn:isbn:" ) == 0 ) {
        isbn = isbn.substr ( 9 );
    }

    isbn.erase ( std::remove_if ( isbn.begin(), isbn.end(),
    [] ( char x ) {
        return !std::isalpha ( x ) && !std::isdigit ( x );
    } ), isbn.end() );

    if ( isbn.find ( "ISBN" ) != std::string::npos ) {
        return isbn.substr ( isbn.find ( "ISBN" ) + 4 );
    } else {
        return isbn;
    }
}

inline std::string clean_track_number( const std::string& track ) {
    std::string _track = ( track.find( "/" ) == std::string::npos ? track : track.substr( 0, track.find( "/" ) ) );
    _track.erase( _track.begin(), std::find_if( _track.begin(), _track.end(), std::bind1st( std::not_equal_to<char>(), '0' ) ) );
    return _track;
}

template< class T >
inline rapidxml_ns::xml_node<>* element( rapidxml_ns::xml_document<>* doc, T* parent, const std::string& name, const std::string& value ) {
    rapidxml_ns::xml_node<> *_n = doc->allocate_node(rapidxml_ns::node_element,
                                                     doc->allocate_string(name.c_str()),
                                                     doc->allocate_string(value.c_str()) );
    parent->append_node(_n);
    return _n;
}
template< class T >
inline void attr( rapidxml_ns::xml_document<>* doc, T* parent, const std::string& name, const std::string& value ) {
    rapidxml_ns::xml_attribute<> *_attr = doc->allocate_attribute( doc->allocate_string(name.c_str()), doc->allocate_string(value.c_str()) );
    parent->append_attribute(_attr);
}
}//namespace cds
#endif // _CDS_UTILS_H
