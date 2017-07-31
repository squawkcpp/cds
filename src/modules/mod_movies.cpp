#include "mod_movies.h"

#include "format.h"
#include "codec.h"

namespace cds {
namespace mod {
void ModMovies::import( data::redox_ptr rdx ) {
    redox::Command<std::unordered_set< std::string > >& c =
        rdx->commandSync<std::unordered_set< std::string > >( { "SMEMBERS", "type:movie" } );

    if( c.ok() ) {
        for( const std::string& __c : c.reply() ) {
            std::map< std::string, std::string > _file = data::node( rdx, __c );
            std::cout << "parse movie:" << _file[PARAM_PATH] << std::endl;

            //Get the track information
            av::Format _format( _file[PARAM_PATH] );
            if( !!_format )
            {
                throw _format.errc();
            }

            auto codec = _format.find_codec( av::CODEC_TYPE::VIDEO );
            rdx->command( {"HMSET",  make_key( KEY_FS, __c ),
//               av::Metadata::name( av::Metadata::ARTIST ), _artist,
//               av::Metadata::name( av::Metadata::ARTIST ), _artist,
//               av::Metadata::name( av::Metadata::ALBUM ), _album,
//               av::Metadata::name( av::Metadata::YEAR ), _year,
//               av::Metadata::name( av::Metadata::TRACK ), _track,
//               av::Metadata::name( av::Metadata::DISC ), _disc,
//               av::Metadata::name( av::Metadata::GENRE ), _genre,
                           KEY_BITRATE, std::to_string( codec->bitrate() ),
                           KEY_BPS, std::to_string( codec->bits_per_sample() ),
                           KEY_CHANNELS, std::to_string( codec->channels() ),
                           KEY_WIDTH, std::to_string( codec->width() ),
                           KEY_HEIGHT, std::to_string( codec->height() )
                          } );
            rdx->command( {"SADD",  "fs:movie:list", __c } );
        }
    }
}
}//namespace mod
}//namespace cds
