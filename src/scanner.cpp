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

#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>

#include "spdlog/spdlog.h"
#include "format.h"
#include "codec.h"

#include "_utils.h"
#include "datastore.h"

#include "modules/mod_albums.h"
#include "modules/mod_ebooks.h"
#include "modules/mod_images.h"
#include "modules/mod_movies.h"
#include "modules/mod_series.h"

#include "filemetaregex.h"

namespace cds {

/** @brief store folder */
inline void save_folder ( data::redis_ptr redis /** @param redis redis database pointer. */,
                          const std::string& key /** @param path path of the node. */,
                          const std::string& name /** @param name name of the node. */,
                          const std::string& parent /** @param parent parent key. */ ) {
    data::node_t _node {
        { data::KEY_NAME, name },
        { data::KEY_PARENT, parent },
        { data::KEY_CLASS, data::NodeType::str ( data::NodeType::folder ) } };
    data::save( redis, key, _node );
}

void Scanner::import_files ( data::redis_ptr redis, const config_ptr config ) {
    //create the content nodes
    for ( auto& __mod : data::menu ) {
        save_folder( redis, __mod[data::KEY_TYPE], __mod[data::KEY_NAME], "/" );
    }

    magic_t _magic = magic_open ( MAGIC_MIME_TYPE );
    magic_load ( _magic, nullptr );

    for ( auto directory : config->media ) {
        data::node_t _node;
        _node[data::KEY_NAME] = filename ( directory, false );
        _node[data::KEY_PARENT] = data::TYPE_FILE;
        _node[data::KEY_PATH] = directory;
        _node[data::KEY_CLASS] = data::NodeType::str ( data::NodeType::folder );
        data::save( redis, data::hash( directory ), _node );
        import_directory( redis, _magic, directory, directory );
    }

    magic_close ( _magic );

    mod::ModAlbums::import ( redis, config );
    mod::ModImages::import ( redis, config );
    mod::ModMovies::import ( redis, config );
    mod::ModSeries::import ( redis, config );
    mod::ModEbooks::import ( redis, config );
}
void Scanner::import_directory ( data::redis_ptr redis, magic_t& _magic, const std::string& parent_key, const std::string& path ) {
    //get files in folder
    boost::filesystem::path _fs_path ( path );
    boost::filesystem::directory_iterator end_itr;

    for ( boost::filesystem::directory_iterator itr ( _fs_path ); itr != end_itr; ++itr ) {
        const std::string _item_filepath = path + "/" + itr->path().filename().string();

        if ( boost::filesystem::is_regular_file ( itr->status() ) ) {

            if( ! data::timestamp( redis, data::hash ( _item_filepath ), boost::filesystem::last_write_time( itr->path() ) ) ) {

                const char* _mime_type = magic_file ( _magic, _item_filepath.c_str() );
                data::node_t _node;
                _node[data::KEY_NAME] = filename ( _item_filepath, false );
                data::NodeType::Enum _type = FileMetaRegex::parse ( _mime_type, _item_filepath.c_str(), _node );
                _node[data::KEY_PARENT] = data::hash( parent_key );
                _node[data::KEY_PATH] = _item_filepath;
                _node[data::KEY_CLASS] = data::NodeType::str ( _type );
                _node[KEY_EXTENSION] = boost::filesystem::extension ( itr->path() );
                _node[KEY_SIZE] = std::to_string ( boost::filesystem::file_size ( itr->path() ) );
                _node[KEY_MIME_TYPE] = _mime_type;

                data::save( redis, data::hash( _item_filepath ), _node );
                data::incr_mime ( redis, _mime_type );
                data::new_item(redis, data::hash ( parent_key ), data::hash ( _item_filepath ), _type );
            }
        } else if ( boost::filesystem::is_directory ( itr->status() ) ) {

            data::node_t _node {
                { data::KEY_NAME, filename ( _item_filepath, false ) },
                { data::KEY_PARENT, data::hash( path ) },
                { data::KEY_PATH, _item_filepath },
                { data::KEY_CLASS, data::NodeType::str ( data::NodeType::folder ) } };
            data::save( redis, data::hash( _item_filepath ), _node );
            import_directory ( redis, _magic, _item_filepath, _item_filepath );
        }
    }
}
}//namespace cds
