#ifndef DATASTORE_H
#define DATASTORE_H

#include <map>
#include <string>
#include <vector>

#include "fmt/format.h"

#include <redox.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "_utils.h"

namespace cds {
/**
 *@brief Read and write CDS data in redis.
 * <h2>Datastructure</h2>
 * <h3>Configuration</h3>
 * <ul>
 * <li>cds:config [string] CDS configuration as json string.</li>
 * </ul>
 *
 * <h3>Filesystem:</h3>
 * <ul>
 * <li>fs:{hash:key} [HASH] The files.</li>
 * <li>fs:{hash:key}:children [SET] The file child nodes.</li>
 * </ul>
 *
 * <h3>Views:</h3>
 * <ul>
 * <li>type:{type name} [SET] the view.</li>
 * </ul>
 */
namespace data {

typedef std::shared_ptr< redox::Redox > redox_ptr;

inline redox_ptr make_connection( const std::string db, int port ) {
    redox_ptr rdx = redox_ptr( new redox::Redox() );
    if( !rdx->connect( db, port ) )
    { throw std::system_error( std::error_code(300, std::generic_category() ), "unable to connect to database on localhost." ); }
    return std::move( rdx );
}

/** @brief get the configuration as json string from the database. */
inline std::string config( redox_ptr rdx /** @param rdx redox database. */ ) {
    return rdx->get( KEY_CONFIG );
}
/** @brief store the configuration as json string in the database. */
inline void config( redox_ptr rdx, /** @param rdx redox database. */
             const std::string& json /** @param c the configuration as json string. */ ) {
    rdx->set( KEY_CONFIG, json );
}
/** @brief check if a configuration exists in the database. */
inline bool config_exists( redox_ptr rdx /** @param rdx redox database. */ ) {
    redox::Command<int>& c = rdx->commandSync<int>({"EXISTS", KEY_CONFIG} );
    return( c.ok() && c.reply() );
}

template< class T >
std::map<std::string, std::string> status(T /*rdx*/) {
    std::map<std::string, std::string> _res;
    _res["nodes"] = "1001"; //TODO
    _res["albums"] = "42"; //TODO
    return _res;
}

/** @brief get the item filesystem path by redis key */
inline std::string path( redox_ptr rdx /** @param rdx redox database. */,
                         const std::string& key /** @param key the item key. */ ) {
    auto& _item = rdx->commandSync< std::string >(
        { "HGET", make_key( KEY_FS, key ), PARAM_PATH } );
    if( ! _item.ok())
    { throw std::system_error( std::error_code(404, std::generic_category() ), "item not found." ); }
    return( _item.reply() );
}



inline void children( redox_ptr rdx, const std::string& key, int /*TODO index*/, int /*TODO count*/,
                           std::vector< std::string > type, std::function< void( const std::string& ) > f ) {

    std::vector< std::string > _command;

    if( is_mod( key ) ) {
        _command = { "SMEMBERS", make_key( mod_key( key ), KEY_LIST ) };
    } else {
        _command.push_back( "SUNION" );
        for( auto& __type : NodeTypes ) {
//            if( std::find(type.begin(), type.end(), str( __type ) ) != type.end() )
//            {  _command.push_back( make_key( str( __type ), KEY_LIST ) ); /** TODO what is the difference? */}
            /* else */ if( type.empty() || std::find(type.begin(), type.end(), str( __type ) ) != type.end() )
            {  _command.push_back( make_key( KEY_FS, key, str( __type ) ) ); }
        }
    }

    redox::Command<std::unordered_set< std::string > >& c =
            rdx->commandSync<std::unordered_set< std::string > >( _command );

    if( c.ok() ) {
        for( const std::string& __key : c.reply() ) {
            f( __key );
        }
    }
}

inline std::map< std::string, std::string > node(
        redox_ptr rdx,
        const std::string& key ) {

    auto& _item = rdx->commandSync<std::vector< std::string > >(
        { "HGETALL", fmt::format( "fs:{}", key ) } );
    if( ! _item.ok())
    { throw std::system_error( std::error_code(300, std::generic_category() ), "unable to get item." ); }
    return( cds::to_map( _item.reply() ) );
}

inline void save_object( data::redox_ptr rdx, const std::string& parent_key, cds::NodeType::Enum type,
                              const std::string& path, std::map< std::string, std::string >& metadata ) {

    //save a temporary with folder related to type.
    rdx->command( {"SADD",  make_key( "tmp", cds::NodeType::name( type ) ), parent_key } );

    //save the item and metadata
    std::vector< std::string > _update_command;
    _update_command.push_back( "HMSET" );
    _update_command.push_back( cds::make_key( KEY_FS, cds::hash( path ) ) );
    _update_command.push_back( cds::PARAM_PARENT );
    _update_command.push_back( cds::hash( parent_key ) );
    _update_command.push_back( cds::PARAM_PATH );
    _update_command.push_back( path );
    _update_command.push_back( cds::PARAM_CLASS );
    _update_command.push_back( cds::NodeType::name( type ) );
    for( auto __m : metadata ) {
        _update_command.push_back( __m.first );
        _update_command.push_back( __m.second );
    }
    rdx->command( _update_command );

    //save relation to parent
    rdx->command( {"SADD",  cds::make_key( KEY_FS, cds::hash( parent_key ), cds::NodeType::name( type ) ), cds::hash( path ) } );
    //save relation to type
    rdx->command( {"SADD",  cds::make_key( "type", cds::NodeType::name( type ) ), cds::hash( path ) } );
}

inline void incr_mime( data::redox_ptr rdx, const std::string& mime_type ) {

    //reference in mime-type set
    rdx->command( { "SADD", "mime", mime_type } );

    //incr the mime_type counter
    std::string _db_key = "mime:";
    _db_key.append( mime_type );
    rdx->command( { "INCR", _db_key } );
}



typedef std::vector< std::string > types_t;

template< class O, class R >
static void load_item(O& out, R& _redox, const std::string& path ) {

    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    writer.StartObject();

    auto _meta = data::node( _redox, path );
    writer.String( PARAM_KEY.c_str() );
    writer.String( path.c_str() );
    for( auto& __item : _meta ) {
        writer.String( __item.first.c_str() );
        writer.String( __item.second.c_str() );
    }
    writer.EndObject();
    out << sb.GetString();
}

template< class O, class R >
static void load_nodes(O& out, R& _redox, const std::string& path, types_t& types ) {

    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    writer.StartObject();
    writer.String( "nodes" );
    writer.StartArray();

    data::children( _redox, path, 0, 100, types, [&writer,_redox]( const std::string& key ) { //TODO pager attributes
      auto _meta = data::node( _redox, key );
      writer.StartObject();
      writer.String( PARAM_KEY.c_str() );
      writer.String( key.c_str() );
      for( auto& __item : _meta ) {
          writer.String( __item.first.c_str() );
          writer.String( __item.second.c_str() );
      }
      writer.EndObject();
    } );

    writer.EndArray();
    writer.EndObject();
    out << sb.GetString();
}

inline types_t split_types( const std::string& in ) {
    types_t _types;
    if( in.find( "," ) != std::string::npos ) {
        boost::char_separator<char> sep(", ");
        boost::tokenizer< boost::char_separator<char> > tokens(in, sep);
        BOOST_FOREACH (const std::string& t, tokens) {
            if( NodeType::valid( t ) )
            { _types.push_back( t ); }
        }
    } else if( NodeType::valid( in ) )
    { _types.push_back( in ); }
    return _types;
}
}//namespace data
}//namespace cds
#endif // DATASTORE_H
