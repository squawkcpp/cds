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

#include <opencv2/opencv.hpp>

#include "../utils/imageexif.h"

namespace cds {
namespace mod {
void ModImages::import ( data::redis_ptr redis, const config_ptr config ) {
    try {
        data::new_items( redis, NodeType::image, [redis,config]( const std::string& key, data::node_t _node ) {

            cv::Mat _image = cv::imread ( _node[PARAM_PATH], CV_LOAD_IMAGE_COLOR );

            //store values
            utils::exif( _node );
            _node[KEY_WIDTH] = std::to_string ( _image.cols );
            _node[KEY_HEIGHT] = std::to_string ( _image.rows );
            _node[ECoverSizes::str ( ECoverSizes::MED )] = scale ( _image, config->tmp_directory, ECoverSizes::MED, key );
            _node[ECoverSizes::str ( ECoverSizes::TN )] = scale ( _image, config->tmp_directory, ECoverSizes::TN, key );
            data::save( redis, key, _node );
            data::add_nodes( redis, NodeType::image, key );

            //save camera keyword
            if( _node.find( PARAM_MAKE ) != _node.end() && !_node[PARAM_MAKE].empty() )
            { data::keyword( redis, PARAM_MAKE, _node[PARAM_MAKE], NodeType::image, key, 0 ); }
            //save parent name keyword
            std::string _name = data::get( redis, _node[PARAM_PARENT], PARAM_NAME );
            data::keyword( redis,
                             PARAM_NAME,
                             _name,
                             NodeType::image, key, 0 );
        });
    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception mod images." );
    }
}
}//namespace mod
}//namespace cds
