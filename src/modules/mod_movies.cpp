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
#include "mod_movies.h"

#include <boost/filesystem.hpp>

#include "format.h"
#include "codec.h"

#include "../_utils.h"
#include "../datastore.h"

namespace cds {
namespace mod {
void ModMovies::import ( data::redis_ptr redis, const config_ptr config ) {

    data::new_items( redis, data::NodeType::movie, [redis,config]( const std::string& key ) {
        data::node_t _node = data::node ( redis, key );

        //Get the track information
        av::Format _format ( _node[data::KEY_PATH] );

        if ( !!_format ) {
            spdlog::get ( LOGGER )->warn ( "Can not open movie file:{} ({})",
                                           _format.errc().message(),
                                           _node[data::KEY_PATH] );
        } else {
            auto codec = _format.find_codec ( av::CODEC_TYPE::VIDEO );
            redis->command ( { data::REDIS_HMSET,  data::make_key_node ( key ),
                KEY_BITRATE, std::to_string ( codec->bitrate() ),
                KEY_BPS, std::to_string ( codec->bits_per_sample() ),
                KEY_CHANNELS, std::to_string ( codec->channels() ),
                KEY_WIDTH, std::to_string ( codec->width() ),
                KEY_HEIGHT, std::to_string ( codec->height() )
            });
            auto _last_write_time = boost::filesystem::last_write_time( data::get( redis, key, data::KEY_PATH ) );
            redis->command ( {data::REDIS_ZADD, data::make_key_list ( data::NodeType::movie ), std::to_string( _last_write_time ), key } );
        }
    });
}
}//namespace mod
}//namespace cds
