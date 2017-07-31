#ifndef _CDS_UTILS_H
#define _CDS_UTILS_H

#include <algorithm>
#include <regex>
#include <map>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/functional/hash.hpp>

#include "fmt/format.h"

namespace cds {

static const std::string LOGGER = "cds";

static const std::string REDIS_ADD = "SADD";
static const std::string REDIS_REM = "SREM";
static const std::string REDIS_SET = "HMSET";
static const std::string REDIS_MEMBERS = "SMEMBERS";

static const std::string KEY_FS = "fs";
static const std::string KEY_LIST = "list";

static const std::string KEY_ARTIST_LIST = "fs:artist:list";
static const std::string KEY_ALBUM_LIST = "fs:album:list";

static const std::string EVENT_SCANNER = "cds:scanner";
static const std::string EVENT_RESCAN = "cds:rescan";

static const std::string VALUE_ROOT = "root";
static const std::string VALUE_FOLDER = "folder";

static const std::string KEY_CONFIG = "cds:config";



static const std::string PARAM_ALBUM = "album";
static const std::string PARAM_ARTIST = "artist";
static const std::string PARAM_AUDIO = "audio";
static const std::string PARAM_AUTHOR = "author";
static const std::string PARAM_BACKDROP = "backdrop";
static const std::string PARAM_COMMENT = "comment";
static const std::string PARAM_COVER = "cover";
static const std::string PARAM_CLASS = "cls";
static const std::string PARAM_DATE = "date";
static const std::string PARAM_EPISODE = "episode";
static const std::string PARAM_EXT = "ext";
static const std::string PARAM_ISBN = "isbn";
static const std::string PARAM_KEY = "key";
static const std::string PARAM_NAME = "name";
static const std::string PARAM_PARENT = "parent";
static const std::string PARAM_PATH = "path";
static const std::string PARAM_POSTER = "poster";
static const std::string PARAM_PUBLISHER = "publisher";
static const std::string PARAM_SEASON = "season";
static const std::string PARAM_SERIE = "serie";
static const std::string PARAM_STILL_IMAGE = "still_image";
static const std::string PARAM_THUMB = "thumb";
static const std::string PARAM_TMDB_ID = "tmdb_id";
static const std::string PARAM_TMP = "tmp";
static const std::string PARAM_TYPE = "type";
static const std::string PARAM_TYPES = "types";
static const std::string PARAM_YEAR = "year";

static const std::string KEY_MIME_TYPE = "mimeType";
static const std::string KEY_BITRATE = "bitrate";
static const std::string KEY_BPS = "bps";
static const std::string KEY_CHANNELS = "channels";
static const std::string KEY_SAMPLERATE = "samplerate";
static const std::string KEY_PLAYTIME = "playlength";
static const std::string KEY_WIDTH = "width";
static const std::string KEY_HEIGHT = "height";
static const std::string KEY_EXTENSION = "ext";
static const std::string KEY_SIZE = "size";

static std::array< std::map< std::string, std::string >, 6 > menu( {{
         { {"name", "Music Albums"},  {"path", PARAM_ALBUM},  {PARAM_KEY, "fs:album"} },
         { {"name", "Music Artists"}, {"path", PARAM_ARTIST}, {PARAM_KEY, "fs:artist"} },
         { {"name", "Photos"},        {"path", "photo"},      {PARAM_KEY, "fs:photo"} },
         { {"name", "Movies"},        {"path", "movie"},      {PARAM_KEY, "fs:movie"} },
         { {"name", "TV Series"},     {"path", PARAM_SERIE},  {PARAM_KEY, "fs:serie"} },
         { {"name", "eBooks"},        {"path", "ebook"},      {PARAM_KEY, "fs:ebook"} }
}} );

/** @brief check if path is a mod path */
static bool is_mod( const std::string& path ) {
    for( auto& __mod : menu ) {
        if( __mod[PARAM_PATH] == path )
        { return true; }
    }
    return false;
}

/** @brief Get the mod database key from the path */
inline std::string mod_key( const std::string& path ) {
    for( auto& __mod : menu ) {
        if( __mod[PARAM_PATH] == path )
        { return __mod[PARAM_KEY]; }
    }
    return "";
}

static std::vector< std::string > trash_words(
    { "flac", "24bit", "720p", "1080p", "amzn", "webrip", "dd5.1", "x264", "mkv",
      "hdtv", "web-dl", "h264", "bdrip", "dts", "bluray", "-hdc", "mp4", "-2hd"
      "-rarbg", "[rarbg]", "-fgt", "-japhson", "-fgt", "-ntb", "-moritz", "-avs",
      "-dumbension", "-it00nz", "-cravers", "-rovers", "[96-24]" }
);

static std::vector< std::string > skip_words(
    { "the ", "der ", "die ", "das ", "24bit" }
);

static std::array< std::string, 3 > album_cover_names(
    { "cover", "front", "folder" }
);

static std::array< std::string, 6 > metadata_relations(
    { "allmusic", "IMDb", "official homepage", "image", "wikipedia", "discogs" }
);

struct ECoverSizes {
    enum Enum { TN, MED };
    static std::string get( const Enum& size ) {
        switch( size ) {
            case TN: return "tn";
            case MED: return "med";
        }
    }
};

inline std::string make_cover_uri( const ECoverSizes::Enum& size, const std::string& key ) {
    return fmt::format( "/cds/{}_{}.jpg", ECoverSizes::get( size ), key );
}

inline std::string make_cover_path( const std::string& path, const ECoverSizes::Enum& size, const std::string& key ) {
    return fmt::format( "{0}/{1}_{2}.jpg", path, size, key );
}

inline std::string remove_skip_list( const std::vector< std::string >& words, const std::string& s ) {
    std::string _return_value = s;
    for( auto& __w : words ) {
        size_t _pos = 0;
        std::string _lower = boost::algorithm::to_lower_copy( _return_value );
        while( ( _pos = _lower.find( __w, _pos ) ) != std::string::npos ) {
            _return_value.erase( _pos, __w.size() );
            _lower.erase( _pos, __w.size() );
        }
    }

    std::regex brace_re("(\\[ *\\])");
    _return_value = std::regex_replace(_return_value, brace_re, "");
    std::regex underscore_re("([_*])");
    _return_value = std::regex_replace(_return_value, underscore_re, " ");
    std::regex dot_re("([\\.*])");
    _return_value = std::regex_replace(_return_value, dot_re, " " );
    boost::algorithm::trim( _return_value );
    return _return_value;
}
inline std::string clean_string ( const std::string& s ) {

    std::string _result = boost::algorithm::to_lower_copy( s );
    for( auto& _word : skip_words ) {
        size_t _position;
        while( ( _position = _result.find( _word ) ) != std::string::npos ) {
            _result.erase( _position, _word.size() );
        }
    }
    boost::algorithm::trim( _result );
    return _result;
}
inline std::string path( const std::string& filename ) {
    std::string _path;
    if( filename.find( "/" ) != std::string::npos )
    { _path = filename.substr( 0, filename.rfind( "/" ) ); }
    else _path = filename;

    return _path;
}
inline std::string filename( const std::string& filename, bool extension = true ) {
    std::string _filename;
    if( filename.find( "/" ) != std::string::npos )
    { _filename = filename.substr( filename.rfind( "/" )+1 ); }
    else _filename = filename;

    if( !extension ) {
        if( _filename.find( "." ) != std::string::npos )
        { _filename.erase( _filename.rfind( "." ) ); }
    }
    return _filename;
}
inline std::string remove_extension( const std::string& file ) {
    if( file.find( "." ) != std::string::npos ) {
        return file.substr( 0, file.rfind( "." ) );
    } else return std::string( file );
}

inline std::string clean_isbn( std::string isbn ) {
    if( isbn.find( "urn:isbn:" ) == 0 ) {
        isbn = isbn.substr( 9 );
    }
    isbn.erase(std::remove_if(isbn.begin(), isbn.end(),
        [](char x){return !std::isalpha(x) && !std::isdigit(x);}), isbn.end());

    if( isbn.find( "ISBN" ) != std::string::npos ) {
        return isbn.substr( isbn.find( "ISBN" ) + 4 );
    } else return isbn;
}

///@cond DOC_INTERNAL
static const std::vector< std::string > __NAMES = std::vector< std::string>(
    { "folder", "audio", "movie", PARAM_SERIE, "image", "ebook", "file", PARAM_ALBUM, "cover", "episode", PARAM_ARTIST } );
///@endcond DOC_INTERNAL

class NodeType {
public:
    enum Enum { folder=0, audio=1, movie=2, serie=3, image=4, ebook=5, file=6, album=7, cover=8, episode=9, artist=10 };

    static std::string name( Enum type )
    { return __NAMES.at( type ); }

    static Enum parse( const std::string& type ) {
        if( type == "folder" ) return folder;
        if( type == "audio" ) return audio;
        if( type == "movie" ) return movie;
        if( type == PARAM_SERIE ) return serie;
        if( type == "image" ) return image;
        if( type == "ebook" ) return ebook;
        if( type == PARAM_ALBUM ) return album;
        if( type == "cover" ) return cover;
        if( type == PARAM_ARTIST ) return artist;
        if( type == "episode" ) return episode;
        return file;
    }

    static bool valid( const std::string& type ) {
        return std::find(__NAMES.begin(), __NAMES.end(), type ) != __NAMES.end();
    }
};
static const std::vector< NodeType::Enum > NodeTypes = std::vector< NodeType::Enum >(
    { NodeType::folder, NodeType::audio, NodeType::movie, NodeType::serie,
      NodeType::image, NodeType::ebook, NodeType::file, NodeType::album,
      NodeType::cover, NodeType::episode, NodeType::artist } );
///@cond DOC_INTERNAL
///@endcond DOC_INTERNAL


inline std::string str( const NodeType::Enum& type )
{ return __NAMES.at( type ); }

static std::map< std::string, std::string > to_map( std::vector< std::string > in ) {
    std::map< std::string, std::string > _map;
    assert( in.size()%2 == 0 );
    for( size_t i=0; i<in.size(); ++++i ) {
        _map[ in.at( i ) ] = in.at( i+1 );
    }
    return _map;
}

/** @brief hash create a hash of the input string. */
inline std::string hash( const std::string& in /** @param in string to hash. */ ) {
    static boost::hash<std::string> _hash;
    if( in == "/" || in == KEY_FS || is_mod( in ) )
    { return in; }
    else
    { return std::to_string( _hash( in ) ); }
}

///@cond DOC_INTERNAL
inline void __iterate_key(
        std::string& key,
        const std::string& value ) {

    key.append( ":" );
    key.append( value );
}
template< class... ARGS >
inline void __iterate_key(
        std::string& key,
        const std::string& value,
        ARGS... args ) {

    key.append( ":" );
    key.append( value );
    __iterate_key( key, args... );
}
///@endcond DOC_INTERNAL
template< typename... ARGS >
/** @brief make_key */
inline std::string make_key(
        const std::string& prefix, /** @param prefix the prefix of the string*/
        const std::string& value, /** @param value the first value to add */
        ARGS... args /** @param args the following parameters */ ) {

    std::string _res = prefix;
    __iterate_key( _res, value, args... );
    return _res;
}
}//namespace cds
#endif // _CDS_UTILS_H
