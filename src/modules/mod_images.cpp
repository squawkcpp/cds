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

#include "../_utils.h"
#include "../datastore.h"
#include "../utils/image.h"
#include "../utils/imageexif.h"

namespace cds {
namespace mod {
void ModImages::import ( data::redis_ptr redis, const config_ptr config, const std::string& key ) {
    try {
        data::node_t _node = data::node( redis, key );
        auto _image = utils::Image( _node[param::PATH] );

        //store values
        //TODO utils::exif( _node );

        _node[param::WIDTH] = std::to_string ( _image.width() );
        _node[param::HEIGHT] = std::to_string ( _image.height() );
        _node[ECoverSizes::str ( ECoverSizes::MED )] = _image.scale( config->tmp_directory, ECoverSizes::MED, key );
        _node[ECoverSizes::str ( ECoverSizes::TN )] = _image.scale( config->tmp_directory, ECoverSizes::TN, key );

        data::save( redis, key, _node );
        data::add_nodes( redis, data::NodeType::image, key, data::time_millis() );

        //save camera keyword
        if( _node.find( param::MAKE ) != _node.end() && !_node[param::MAKE].empty() )
        { data::add_tag( redis, data::NodeType::image, param::MAKE, _node[param::MAKE], key, 0 ); }
        //save parent name keyword
        data::add_tag( redis, data::NodeType::image, param::NAME, data::get( redis, _node[param::PARENT], param::NAME ), key, 0 );
    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception mod images." );
    }
}
}//namespace mod
}//namespace cds
