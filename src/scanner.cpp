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
#include "scanner.h"

#include <boost/filesystem.hpp>

#include "spdlog/spdlog.h"

#include "_utils.h"
#include "datastore.h"
#include "filemetaregex.h"
#include "_lua.h"

#include "modules/mod_albums.h"
#include "modules/mod_ebooks.h"
#include "modules/mod_images.h"
#include "modules/mod_movies.h"
#include "modules/mod_series.h"

namespace cds {

const std::array< std::regex, 3 > Scanner::_disc_patterns {
    {std::regex("(/.*)/CD[0-9]*", std::regex_constants::icase ),
    std::regex("(/.*)/DISC[0-9]*", std::regex_constants::icase ),
    std::regex("(/.*)/DISK[0-9]*", std::regex_constants::icase )}
};

void Scanner::import_files ( data::redis_ptr redis, const config_ptr config ) {

    magic_t _magic = magic_open ( MAGIC_MIME_TYPE );
    magic_load ( _magic, nullptr );

    for ( auto directory : config->media ) {
        data::save( redis, data::hash( directory ), {
            { param::NAME, filename ( directory, false ) },
            { param::PARENT, param::FILE },
            { param::PATH, directory },
            { param::CLASS, data::NodeType::str ( data::NodeType::folder ) },
            { param::TIMESTAMP, "0" },
        });
        data::add_types( redis, param::FILE, data::hash( directory ), boost::filesystem::last_write_time( directory ) );
        import_directory( redis, _magic, directory, directory );
    }
    magic_close ( _magic );

    Scanner::new_items( redis, config, data::NodeType::audio, std::bind( &mod::ModAlbums::import, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
//    Scanner::new_items( redis, config, data::NodeType::image, std::bind( &mod::ModImages::import, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
//    Scanner::new_items( redis, config, data::NodeType::movie, std::bind( &mod::ModMovies::import, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
//    Scanner::new_items( redis, config, data::NodeType::episode, std::bind( &mod::ModSeries::import, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
//    Scanner::new_items( redis, config, data::NodeType::ebook, std::bind( &mod::ModEbooks::import, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3 ) );
    Scanner::sweep( redis, param::FILE );
    data::eval( redis, LUA_INDEX, 0 );
    search_index( redis );
}
void Scanner::import_directory ( data::redis_ptr redis, magic_t& _magic, const std::string& parent_key, const std::string& path ) {
    //get files in folder
    boost::filesystem::path _fs_path ( path );
    boost::filesystem::directory_iterator end_itr;

    for ( boost::filesystem::directory_iterator itr ( _fs_path ); itr != end_itr; ++itr ) {
        const std::string _item_filepath = path + "/" + itr->path().filename().string();
        if ( boost::filesystem::is_regular_file ( itr->status() ) ) {
            if( ! Scanner::timestamp( redis, data::hash ( _item_filepath ), boost::filesystem::last_write_time( itr->path() ) ) ) {
                const char* _mime_type = magic_file ( _magic, _item_filepath.c_str() );
                data::node_t _node;
                data::NodeType::Enum _type = FileMetaRegex::parse ( _mime_type, _item_filepath.c_str(), _node );
                _node[param::CLEAN_STRING] = clean_string( _node[param::NAME] );
                _node[param::PARENT] = data::hash( parent_key );
                _node[param::PATH] = _item_filepath;
                _node[param::CLASS] = data::NodeType::str ( _type );
                _node[param::EXTENSION] = boost::filesystem::extension ( itr->path() );
                _node[param::SIZE] = std::to_string ( boost::filesystem::file_size ( itr->path() ) );
                _node[param::MIME_TYPE] = _mime_type;
                _node[param::TIMESTAMP] = std::to_string( boost::filesystem::last_write_time( itr->path() ) );
                data::save( redis, data::hash( _item_filepath ), _node );
                data::add_types( redis, data::hash( parent_key ), data::hash( _item_filepath ),  data::time_millis() );
                data::incr_mime ( redis, _mime_type );
                Scanner::new_item(redis, parent_key, data::hash ( _item_filepath ), _type );
            }
        } else if ( boost::filesystem::is_directory ( itr->status() ) ) {
            if( ! Scanner::timestamp( redis, data::hash ( _item_filepath ), boost::filesystem::last_write_time( itr->path() ) ) ) {
                data::save( redis, data::hash( _item_filepath ), {
                    { param::NAME, filename ( _item_filepath, false ) },
                    { param::CLEAN_STRING, clean_string( filename ( _item_filepath, false ) ) },
                    { param::PARENT, data::hash( path ) },
                    { param::PATH, _item_filepath },
                    { param::CLASS, data::NodeType::str ( data::NodeType::folder ) },
                    { param::TIMESTAMP, std::to_string( boost::filesystem::last_write_time( itr->path() ) ) }
                });
                data::add_types( redis, data::hash( parent_key ), data::hash( _item_filepath ),  data::time_millis() );
            }
            import_directory ( redis, _magic, _item_filepath, _item_filepath );
        }
    }
}

void Scanner::new_item( data::redis_ptr redis, const std::string& parent, const std::string& key, const data::NodeType::Enum type ) {
    if( type == data::NodeType::audio ) {
        redis->command( {redis::SADD,
                         data::make_key( key::FS, key::NEW, data::NodeType::str( type ) ),
                         data::hash( Scanner::remove_disc( parent ) ) } );
    } else {
        redis->command( {redis::SADD,  data::make_key( key::FS, key::NEW, data::NodeType::str( type ) ), key} );
    }
}

void Scanner::sweep ( data::redis_ptr redis, const std::string& key ) {
    data::children( redis, key, 0, -1, "default", "asc", "", [redis,key]( const std::string& item ) {
        sweep( redis, item );

        if( !data::is_mod( item ) && item != param::ROOT ) {
            const std::string _item_path = data::get( redis, item, param::PATH );
            if( !boost::filesystem::exists( _item_path ) ) {
                SPDLOG_DEBUG(spdlog::get ( LOGGER ), "orphan found (key={}, path={})", item, _item_path );
                const std::string _item_class = data::get( redis, item, param::CLASS );

                //TODO Scanner::flush( redis, data::make_key_node( item ) );
                redis->command ( {redis::SREM, data::make_key_list( key ), item } );
                //TODO delete from sort nodes
                if( data::is_mod( _item_class ) ) {

                    //flush all the child elments
                }
            }
        }

    });
}

bool Scanner::timestamp( data::redis_ptr redis, const std::string& key, unsigned long timestamp ) {
    auto& _exist = redis->commandSync< int > ( { redis::EXISTS, data::make_key_node ( key ) } );
    if( _exist.ok() && _exist.reply() == 1 ) {
        auto _timestamp = std::stoul( data::get( redis, key, param::TIMESTAMP ) );
        if( _exist.ok() && _timestamp == timestamp ) { return true; }
    }
    return false;
}

std::string Scanner::remove_disc( const std::string& path ) {
    for( auto& _regex : Scanner::_disc_patterns ) {
        std::smatch matches;
        if( std::regex_search( path, matches, _regex ) ) {
            return matches[1];
        }
    }
    return path;
}
void Scanner::new_items ( data::redis_ptr redis, const config_ptr config, data::NodeType::Enum type,
                        std::function< void(data::redis_ptr, const config_ptr, const std::string&) > fn ) {

    auto& _c = redis->commandSync< data::nodes_t >(
        {redis::SMEMBERS, data::make_key( key::FS, key::NEW, data::NodeType::str( type ) ) } );

    if( _c.ok() ) {
        for( const std::string& __c : _c.reply() ) {
            fn( redis, config, __c );
            redis->command( {redis::SREM, data::make_key( key::FS, key::NEW, data::NodeType::str( type ) ), __c } );
        }
    }
}

/** @brief flush content with the given prefix. */
void Scanner::flush( data::redis_ptr redis /** @param redis the database pointer. */ ) {
    data::eval ( redis, LUA_FLUSH, 0, "fs:*" );
    //create the content nodes
    save_folder( redis, param::ROOT, param::ROOT, "" );
    for ( auto& __mod : _internal::menu ) {
        save_folder( redis, __mod.at( key::TYPE ), __mod.at( param::NAME ), param::ROOT );
        data::add_types( redis, param::ROOT, __mod.at( key::TYPE ), data::time_millis() );
    }
}

/** @brief store folder */
void Scanner::save_folder ( data::redis_ptr redis /** @param redis redis database pointer. */,
                          const std::string& key /** @param path path of the node. */,
                          const std::string& name /** @param name name of the node. */,
                          const std::string& parent /** @param parent parent key. */ ) {
    data::node_t _node {
        { param::NAME, name },
        { param::PARENT, parent },
        { param::CLASS, data::NodeType::str ( data::NodeType::folder ) },
        { param::TIMESTAMP, std::to_string( data::time_millis() ) },
    };
    data::save( redis, key, _node );
}
void Scanner::search_index ( data::redis_ptr redis /** @param redis redis database pointer. */ ) {

    redis->command( {"FT.DROP", "idx" } );
    redis->command( {"FT.CREATE", "idx", "SCHEMA", "name", "TEXT", "WEIGHT", "5.0", "artist", "TEXT", "album", "TEXT" } );
    //TODO add genre

    data::children( redis, param::ALBUM, 0, -1, "default", "asc", "", [redis]( const std::string item ) {
        redis->command( {"FT.ADD", "idx", item, "1.0", "FIELDS",
            param::NAME, data::get( redis, item, param::NAME ),
            param::ARTIST, data::get( redis, item, param::ARTIST ),
            param::ALBUM, data::get( redis, item, param::NAME ) } );
        redis->command( {"FT.SUGADD", "autocomplete", data::get( redis, item, param::NAME ), "100" } );
        redis->command( {"FT.SUGADD", "autocomplete", data::get( redis, item, param::ARTIST ), "100" } );
    });
}

}//namespace cds
