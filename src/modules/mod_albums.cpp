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
#include "mod_albums.h"

#include <boost/filesystem.hpp>

#include "spdlog/spdlog.h"

#include "format.h"
#include "codec.h"

#include "rapidxml_ns.hpp"

#include "../_utils.h"
#include "../datastore.h"
#include "../utils/image.h"

#include "http/httpclient.h"

//TODO store genre as tag

namespace cds {
namespace mod {
void ModAlbums::import ( data::redis_ptr redis, const config_ptr config ) {
    data::new_items( redis, data::NodeType::audio, [redis,config]( const std::string& key ) {
            std::map< data::NodeType::Enum, std::vector< data::node_t > > _files;
            import ( redis, key, _files );
            std::string _artist = "", _album = "", _year = "", _track = "", _disc = "", _genre = "", _title = "";
            bool _cover_found = false;
            std::string _last_cover = "";

            for ( auto& __type : _files ) {
                if ( __type.first == data::NodeType::audio ) {
                    for ( auto& __file : __type.second ) {
                        _artist = __file[av::Metadata::name ( av::Metadata::ARTIST )];
                        _album = __file[av::Metadata::name ( av::Metadata::ALBUM )];
                        _year = __file[av::Metadata::name ( av::Metadata::YEAR )];
                        _track = clean_track_number( __file[av::Metadata::name ( av::Metadata::TRACK )] );
                        _disc = __file[av::Metadata::name ( av::Metadata::DISC )];
                        //Get the track information
                        av::Format _format ( __file[data::KEY_PATH] );

                        if ( !!_format ) {
                            spdlog::get ( LOGGER )->warn ( "Can not open audio file:{} ({})",
                                                           _format.errc().message(),
                                                           __file[data::KEY_PATH] );
                        } else {
                            av::Metadata _metadata = _format.metadata();

                            if ( !_metadata.get ( av::Metadata::TITLE ).empty() )
                            { _title = _metadata.get ( av::Metadata::TITLE ); }

                            if ( !_metadata.get ( av::Metadata::ARTIST ).empty() )
                            { _artist = _metadata.get ( av::Metadata::ARTIST ); }

                            if ( !_metadata.get ( av::Metadata::ALBUM ).empty() )
                            { _album = _metadata.get ( av::Metadata::ALBUM ); }

                            if ( !_metadata.get ( av::Metadata::YEAR ).empty() )
                            { _year = _metadata.get ( av::Metadata::YEAR ); }

                            if ( !_metadata.get ( av::Metadata::TRACK ).empty() )
                            { _track = _metadata.get ( av::Metadata::TRACK ); }

                            if ( !_metadata.get ( av::Metadata::DISC ).empty() )
                            { _disc = _metadata.get ( av::Metadata::DISC ); }

                            if ( !_metadata.get ( av::Metadata::GENRE ).empty() )
                            { _genre = _metadata.get ( av::Metadata::GENRE ); }

                            auto codec = _format.find_codec ( av::CODEC_TYPE::AUDIO );
                            redis->command ( {data::REDIS_HMSET,  data::make_key_node ( data::hash ( __file[data::KEY_PATH] ) ),
                                            data::KEY_PARENT, key,
                                            av::Metadata::name ( av::Metadata::ARTIST ), _artist,
                                            av::Metadata::name ( av::Metadata::ALBUM ), _album,
                                            av::Metadata::name ( av::Metadata::YEAR ), _year,
                                            av::Metadata::name ( av::Metadata::TRACK ), _track,
                                            av::Metadata::name ( av::Metadata::DISC ), _disc,
                                            av::Metadata::name ( av::Metadata::GENRE ), _genre,
                                            KEY_BITRATE, std::to_string ( codec->bitrate() ),
                                            KEY_BPS, std::to_string ( codec->bits_per_sample() ),
                                            KEY_CHANNELS, std::to_string ( codec->channels() ),
                                            KEY_SAMPLERATE, std::to_string ( codec->sample_rate() ),
                                            KEY_PLAYTIME, std::to_string ( _format.playtime() )
                                           } );

                            //update relation with track as score
                            redis->command ( {data::REDIS_ZADD, data::make_key_list ( key ), std::to_string( data::time_millis() ), data::hash ( __file[data::KEY_PATH] ) } );
                        }
                    }

                } else if ( __type.first == data::NodeType::image ) {
                    for ( auto& __file : __type.second ) {
                        utils::Image image_meta_ ( __file[data::KEY_PATH] );
                        redis->command ( {data::REDIS_HMSET,  data::make_key_node ( data::hash ( __file[data::KEY_PATH] ) ),
                                        data::KEY_PARENT, key,
                                        data::KEY_CLASS, data::NodeType::str ( data::NodeType::cover ),
                                        KEY_WIDTH, std::to_string ( image_meta_.width() ),
                                        KEY_HEIGHT, std::to_string ( image_meta_.height() )
                                       } );
                        auto _filename = filename ( __file[data::KEY_PATH], false );
                        boost::to_lower ( _filename );

                        //store cover uri in redis map
                        _last_cover = utils::make_cover_uri ( ECoverSizes::TN, data::hash ( __file[data::KEY_PATH] ) );
                        if ( std::find (  album_cover_names.begin(),  album_cover_names.end(), _filename ) !=  album_cover_names.end() ) {
                            _cover_found = true;
                            redis->command ( { data::REDIS_HMSET, data::make_key_node ( key ), PARAM_THUMB, _last_cover } );
                        }

                        //scale image
                        image_meta_.scale ( config->tmp_directory, ECoverSizes::MED, data::hash( __file[data::KEY_PATH] ) );
                        image_meta_.scale ( config->tmp_directory, ECoverSizes::TN, data::hash( __file[data::KEY_PATH] ) );

                        data::add_types( redis, key, data::hash( __file[data::KEY_PATH] ), 0 );
                        data::rem_types( redis, key, data::hash( __file[data::KEY_PATH] ) );
                        data::rem_nodes( redis, data::NodeType::image, data::hash( __file[data::KEY_PATH] ) );
                    }
                } else if ( __type.first == data::NodeType::ebook ) {
                    for ( auto& __file : __type.second ) {
                        redis->command( {data::REDIS_REM,
                            data::make_key( data::KEY_FS, data::KEY_NEW, data::NodeType::str( data::NodeType::ebook ) ),
                            data::hash( __file[data::KEY_PATH] ) } );
                    }
                } else { spdlog::get ( LOGGER )->debug ( "OTHER TYPE: {}", data::NodeType::str ( __type.first ) ); }
            }

            //when no cover is found try to take another image
            if( !_cover_found && !_last_cover.empty() )
            { redis->command ( { data::REDIS_HMSET, data::make_key_node ( key ), PARAM_THUMB, _last_cover } ); }

            //store folder as album
            redis->command ( {data::REDIS_HMSET,  data::make_key_node ( key ),
                            data::KEY_CLASS, data::NodeType::str ( data::NodeType::album ),
                            data::KEY_NAME, _album,
                            data::KEY_CLEAN_STRING, clean_string( _album ),
                            av::Metadata::name ( av::Metadata::ARTIST ), _artist,
                            av::Metadata::name ( av::Metadata::YEAR ), _year,
                            av::Metadata::name ( av::Metadata::GENRE ), _genre
            } );
            //create genre tag for album
            data::add_tag( redis, "genre", _genre, data::NodeType::movie, key, 0 );

            //add album with timestamp as score
            redis->command ( {data::REDIS_ZADD, data::make_key_list ( data::NodeType::album ), std::to_string( data::time_millis() ), key } );
            import ( redis, key, _artist );
        });
}

void ModAlbums::import ( data::redis_ptr rdx, const std::string& key, std::map< data::NodeType::Enum, std::vector< data::node_t > >& files ) {

    auto& c = rdx->commandSync< data::nodes_t > (
        { data::REDIS_ZRANGE, data::make_key_list ( key ), "0", "-1" } );

    if ( c.ok() ) {
        for ( const std::string& __key : c.reply() ) {
            data::node_t _file = data::node ( rdx, __key );

            if ( _file[data::KEY_CLASS] == data::NodeType::str ( data::NodeType::folder ) ) {
                import ( rdx, data::hash( _file[data::KEY_PATH] ), files );
                //remove folder and relations
                data::rem_types( rdx, key, _file[data::KEY_PATH] );
                rdx->command( {data::REDIS_DEL,
                    data::make_key_list( data::hash( _file[data::KEY_PATH] ) ),
                    data::make_key_node( data::hash( _file[data::KEY_PATH] ) ) } );

            } else { files[ data::NodeType::parse ( _file[data::KEY_CLASS] ) ].push_back ( _file ); }
        }
    }
}

/** @brief import the artist */
void ModAlbums::import ( data::redis_ptr rdx, const std::string& album_key, const std::string& artist ) {

    auto _clean_string = clean_string ( artist );
    rdx->command ( { data::REDIS_HMSET,  data::make_key_node ( _clean_string ), //TODO hash
                     data::KEY_CLASS, data::NodeType::str ( data::NodeType::artist ),
                     data::KEY_PARENT, album_key,
                     data::KEY_NAME, artist,
                     data::KEY_TIMESTAMP, _clean_string } );

    rdx->command ( {data::REDIS_ZADD, data::make_key_list( data::NodeType::str( data::NodeType::artist ) ), std::to_string( data::time_millis() ), _clean_string } );
    rdx->command ( {data::REDIS_ZADD, data::make_key_list( _clean_string ), std::to_string( data::time_millis() ), album_key } );
}
}//namespace mod
}//namespace cds
