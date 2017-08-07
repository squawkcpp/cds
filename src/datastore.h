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
#ifndef DATASTORE_H
#define DATASTORE_H

#include <map>
#include <string>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>

#include "fmt/format.h"
#include <redox.hpp>
#include "spdlog/spdlog.h"

#include "_utils.h"

namespace cds {

/** @brief Read and write CDS data in redis.
<h2>Datastructure</h2>
<h3>Configuration</h3>
<ul>
<li>TODO cds:config [string] CDS configuration as json string.</li>
</ul>

<h3>Node Types:</h3>
<p>the node types like artist or album ar defined in cds::NodeType.</p>

<h3>Structure:</h3>
<p>typically the root node is the start node. the client will navigate from there.</p>
<ul>
<li>fs:KEY [HASH] CDS root node.</li>
<li>fs:KEY:TYPE [SET] child nodes of the root node.</li>
</ul>

<li>fs:/:TYPE [SET] child nodes of the root node.</li>
<li>fs:/:TYPE [SET] child nodes of the root node.</li>
 */
namespace data {

typedef std::shared_ptr< redox::Redox > redis_ptr;
typedef std::map< std::string, std::string > node_t;
typedef std::vector< std::string > types_t; //TODO remove
typedef std::unordered_set< std::string > nodes_t;

/* database utils */

/** @brief Make redis connection. */
inline redis_ptr make_connection ( const std::string db /** @param db database host */,
                                   int port /** @param port database port */ ) {
    redis_ptr _redis = redis_ptr ( new redox::Redox() );

    if ( !_redis->connect ( db, port ) ) {
        throw std::system_error ( std::error_code ( 300, std::generic_category() ),
                                  "unable to connect to database on localhost." );
    }

    return std::move ( _redis );
}

/** @brief make node key */
inline std::string make_key_node ( const std::string& key /** @param key node key. */ ) {
    return make_key ( KEY_FS, key );
}
/** @brief make type list key (fs:KEY:TYPE (SET:KEY) ) */
inline std::string make_key_types ( const std::string& key /** @param key node key. */,
                                    const NodeType::Enum& type /** @param type node type. */ ) {
    return cds::make_key ( KEY_FS, key, cds::NodeType::str ( type ) );
}
/** @brief make node list key (fs:TYPE:list (KEY) ) */
inline std::string make_key_nodes ( const NodeType::Enum& type /** @param type node type. */ ) {
    return cds::make_key ( KEY_FS, cds::NodeType::str ( type ), KEY_LIST );
}
/** @brief make type list key (fs:TYPE:parent (PARENT KEY) ) */
inline std::string make_key_folders ( const NodeType::Enum& type /** @param type node type. */ ) {
    return make_key ( KEY_FS, cds::NodeType::str ( type ), PARAM_PARENT );
}

inline std::string get( redis_ptr redis, const std::string& key, const std::string param ) {

    auto& _item = redis->commandSync< std::string > (
        {REDIS_HGET, data::make_key_node ( key ), param} );

    if ( _item.ok() )
        return _item.reply();

    return "NaN";
}

template< class Fn >
inline void nodes( redis_ptr redis, cds::NodeType::Enum type, Fn fn ) {
    redox::Command< nodes_t >& _c = redis->commandSync< nodes_t >( {cds::REDIS_MEMBERS, make_key_nodes( type ) } );
    if( _c.ok() ) {
        for( const std::string& __c : _c.reply() ) {
            fn( __c );
        }
    }
}

/** @brief add node to parents type list */
inline void add_types( redis_ptr redis, cds::NodeType::Enum type, const std::string& parent, const std::string& key )
{ redis->command( {REDIS_ADD,  data::make_key_types( parent, type ), hash( key ) } ); }
/** @brief remove node from parents type list */
inline void rem_types( redis_ptr redis, cds::NodeType::Enum type, const std::string& parent, const std::string& key )
{ redis->command( {REDIS_REM,  data::make_key_types( parent, type ), hash( key ) } ); }

/** @brief add node to global nodes list */
inline void add_nodes( redis_ptr redis, cds::NodeType::Enum type, const std::string& key )
{ redis->command( {REDIS_ADD,  data::make_key_nodes( type ), key } ); }
/** @brief remove node from global nodes list */
inline void rem_nodes( redis_ptr redis, cds::NodeType::Enum type, const std::string& key )
{ redis->command( {REDIS_REM,  data::make_key_nodes( type ), key } ); }

/** @brief count nodes in set. */
inline size_t count ( redis_ptr redis /** @param redis redis database pointer. */,
                      const std::string& key /** @param key the node key. */ ) {

    auto& _item = redis->commandSync<int> ( { REDIS_SCARD, key } );
    if ( !_item.ok() )
    { return 0; }
    return ( _item.reply() );
}

/** @brief get the node by key */
inline node_t node ( redis_ptr redis /** @param redis redis database pointer. */,
                     const std::string& key /** @param key the node key. */ ) {

    auto& _item = redis->commandSync< types_t > ( { REDIS_HGETALL, make_key_node ( key ) } );
    if ( !_item.ok() ) return {};
    return ( cds::to_map ( _item.reply() ) );
}

inline void save( redis_ptr redis, const std::string& key, node_t& node ) {
    types_t _command = { REDIS_SET, make_key_node ( key ) };
    for ( auto __i : node ) {
        _command.push_back ( __i.first );
        _command.push_back ( __i.second );
    }
    redis->command ( _command );
    add_types( redis, NodeType::parse( node[PARAM_TYPE] ), node[PARAM_PARENT], key );
}

//TODO
template< class Fn>
/** @brief FOR_NODE iterate all the nodes and call lambda function with node. */
inline void FOR_NODE ( redis_ptr redis /** @param redis redis database pointer. */ ,
                       auto command /** @param command redis command */,
                       Fn fn /** @param fn method called with the node. */ ) {

    redox::Command< nodes_t >& c = redis->commandSync< nodes_t > ( command );
    if ( c.ok() ) {
        for ( const std::string& __key : c.reply() ) {
            fn ( __key, data::node ( redis, __key ) );
        }
    }
}


/** @brief get the item filesystem path by redis key */
inline void keyword( redis_ptr redis /** @param redis redis database pointer. */,
                            const std::string& name /** @param name name of the keyword */,
                            const std::string& keyword /** @param the keyword */,
                            const NodeType::Enum& type /** @param type the type for the keyword */,
                            const std::string& node /** @param node the node key */,
                            float score /** @param score the score of the node index */ ) {

    //TODO string
    redis->command( { "ZADD", make_key( KEY_FS, NodeType::str( type ), "keyword", "list" ), "0", name } );
    redis->command( { "ZADD", make_key( KEY_FS, NodeType::str( type ), "keyword", name ), "0", keyword } );
    redis->command( { "HMSET", make_key( KEY_FS, NodeType::str( type ), "keyword" ), keyword, node } );
}

//TODO
inline bool timestamp( redis_ptr redis, const std::string& key, size_t timestamp ) {
    auto& _exist = redis->commandSync< int > ( { "EXISTS", make_key ( KEY_FS, key, "timestamp" ) } );
    if( _exist.ok() && _exist.reply() == 1 ) {
        size_t _timestamp = std::stoul( redis->get( make_key ( KEY_FS, key, "timestamp" ) ) );
        if(  _timestamp == timestamp )
        { return true; }
    }
    redis->set( make_key ( KEY_FS, key, "timestamp" ), std::to_string( timestamp ) );
    return false;
}
inline void new_item( redis_ptr redis, const std::string& parent, const std::string& key, const NodeType::Enum type ) {
    if( type == NodeType::audio ) {
        redis->command( {REDIS_ADD,  make_key( KEY_FS, "new", NodeType::str( type ) ), parent} );
    } else {
        redis->command( {REDIS_ADD,  make_key( KEY_FS, "new", NodeType::str( type ) ), key} );
    }
}
inline void new_items( redis_ptr redis, const NodeType::Enum type, auto fn ) {

    redox::Command< nodes_t >& _c = redis->commandSync< nodes_t >(
        {cds::REDIS_MEMBERS, make_key( KEY_FS, "new", NodeType::str( type ) ) } );

    if( _c.ok() ) {
        for( const std::string& __c : _c.reply() ) {
            fn( __c,  data::node ( redis, __c ) );
            redis->command( {REDIS_REM, make_key( KEY_FS, "new", NodeType::str( type ) ), __c } );
        }
    }
}


/** @brief get the item filesystem path by redis key */
inline std::string path ( redis_ptr redis /** @param redis redis database pointer. */,
                          const std::string& key /** @param key the item key. */ ) {
    auto& _item = redis->commandSync< std::string > (
    { REDIS_HGET, make_key_node ( key ), PARAM_PATH } );

    if ( _item.ok() ) {
        return ( _item.reply() );
    }

    return "";
}

/** @brief add mime type to list and increment mime counter. */
inline void incr_mime ( data::redis_ptr redis /** @param redis redis database pointer. */,
                        const std::string& mime_type /** @param mime_type the mime type.*/ ) {
    //reference in mime-type set
    redis->command ( { REDIS_ADD, KEY_MIME_LIST, mime_type } );
    //incr the mime_type counter
    redis->command ( { "INCR", make_key ( KEY_FS, KEY_MIME, mime_type ) } );
}

/* nethods to handle configuration */

/** @brief get the configuration as json string from the database. */
inline std::string config ( redis_ptr redis /** @param redis redox database. */ ) {
    return redis->get ( KEY_CONFIG );
}
/** @brief store the configuration as json string in the database. */
inline void config ( redis_ptr redis, /** @param redis redox database. */
                     const std::string& json /** @param c the configuration as json string. */ ) {
    redis->set ( KEY_CONFIG, json );
}
/** @brief check if a configuration exists in the database. */
inline bool config_exists ( redis_ptr redis /** @param redis redox database pointer. */ ) {
    redox::Command<int>& c = redis->commandSync<int> ( { "EXISTS", KEY_CONFIG} );
    return ( c.ok() && c.reply() );
}

/* node persist methods  */

///** @brief store node and create relations. */
//inline void save_object ( data::redis_ptr redis /** @param redis redox database pointer. */,
//                          const std::string& parent_key, /** @param parent_key key of the parent node. */
//                          cds::NodeType::Enum type /** @param type type of the node. */,
//                          const std::string& path /** @param path path of the node */,
//                          node_t& metadata /** @param metadata metadata of this node. */ ) {

//    //save the item and metadata
//    types_t _update_command = {
//        REDIS_SET, make_key_node ( cds::hash ( path ) ),
//        cds::PARAM_PARENT, cds::hash ( parent_key ),
//        cds::PARAM_PATH, path,
//        cds::PARAM_CLASS, cds::NodeType::str ( type )
//    };

//    for ( auto __m : metadata ) {
//        _update_command.push_back ( __m.first );
//        _update_command.push_back ( __m.second );
//    }

//    redis->command ( _update_command );

//    //create index
//    //TODO redis->command ( { REDIS_ADD, make_key_types ( cds::hash ( parent_key ), type ), cds::hash ( path ) } );
//    add_types( redis, type, parent_key, cds::hash ( path ) );
//    //TODO redis->command ( { REDIS_ADD, make_key_nodes ( type ), cds::hash ( path ) } );
//    //TODO add_nodes( redis, type, cds::hash( path ) );
//    //TODO redis->command ( { REDIS_ADD, make_key_folders ( type ), hash ( parent_key ) } );
//}

/* util methods  */

/** @brief split comma separated type list. */
inline types_t split_types ( const std::string& in /** @param types string */ ) {
    types_t _types;

    if ( in.find ( "," ) != std::string::npos ) {
        boost::char_separator<char> sep ( ", " );
        boost::tokenizer< boost::char_separator<char> > tokens ( in, sep );
        BOOST_FOREACH ( const std::string & t, tokens ) {
            if ( NodeType::valid ( t ) ) {
                _types.push_back ( t );
            }
        }
    } else if ( NodeType::valid ( in ) ) {
        _types.push_back ( in );
    }

    return _types;
}
}//namespace data
}//namespace cds
#endif // DATASTORE_H
