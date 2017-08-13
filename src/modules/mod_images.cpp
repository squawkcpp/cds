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
#include "mod_images.h"

#include "../utils/image.h"
#include "../utils/imageexif.h"

namespace cds {
namespace mod {
void ModImages::import ( data::redis_ptr redis, const config_ptr config ) {
    try {
        data::new_items( redis, NodeType::image, [redis,config]( const std::string& key ) {
            data::node_t _node = data::node( redis, key );
            auto _image = utils::Image( _node[PARAM_PATH] );

            //store values
            utils::exif( _node );

            _node[KEY_WIDTH] = std::to_string ( _image.width() );
            _node[KEY_HEIGHT] = std::to_string ( _image.height() );
            _node[ECoverSizes::str ( ECoverSizes::MED )] = _image.scale( config->tmp_directory, ECoverSizes::MED, key );
            _node[ECoverSizes::str ( ECoverSizes::TN )] = _image.scale( config->tmp_directory, ECoverSizes::TN, key );

            data::save( redis, key, _node );
            data::add_nodes( redis, NodeType::image, key );

            //save camera keyword
            if( _node.find( PARAM_MAKE ) != _node.end() && !_node[PARAM_MAKE].empty() )
            { data::keyword( redis, PARAM_MAKE, _node[PARAM_MAKE], NodeType::image, key, 0 ); }
            //save parent name keyword
            data::keyword( redis, PARAM_NAME, data::get( redis, _node[PARAM_PARENT], PARAM_NAME ), NodeType::image, key, 0 );
        });
    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception mod images." );
    }
}
}//namespace mod
}//namespace cds
