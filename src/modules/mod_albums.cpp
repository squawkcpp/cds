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

namespace cds {
namespace mod {
void ModAlbums::import ( data::redis_ptr redis, const config_ptr config, const std::string& key ) {
    try {
        std::map< data::NodeType::Enum, std::vector< data::node_t > > _files;
        import_files ( redis, key, _files );
        std::string _title, _album, _artist, _composer, _performer,
                    _comment, _year, _track, _disc, _genre, _publisher;
        bool _cover_found = false;
        std::string _last_cover = "";

        for ( auto& __type : _files ) {
            if ( __type.first == data::NodeType::audio ) {
                for ( auto& __file : __type.second ) {
                    _title = __file[param::TITLE];
                    _artist = __file[param::ARTIST];
                    _album = __file[param::ALBUM];
                    _year = __file[param::YEAR];
                    _track = clean_track_number( __file[param::TRACK] );
                    _disc = __file[param::DISC];

                    //Get the track information
                    av::Format _format ( __file[param::PATH] );
                    if ( !!_format ) {
                        spdlog::get ( LOGGER )->warn ( "Can not open audio file:{} ({})",
                                                       _format.errc().message(),
                                                       __file[param::PATH] );
                    } else {
                        av::Metadata _metadata = _format.metadata();

                        if ( !_metadata.get ( av::Metadata::TITLE ).empty() )
                        { _title = _metadata.get ( av::Metadata::TITLE ); }

                        if ( !_metadata.get ( av::Metadata::ALBUM ).empty() )
                        { _album = _metadata.get ( av::Metadata::ALBUM ); }

                        if ( !_metadata.get ( av::Metadata::ARTIST ).empty() )
                        { _artist = _metadata.get ( av::Metadata::ARTIST ); }

                        if ( !_metadata.get ( av::Metadata::COMPOSER ).empty() )
                        { _composer = _metadata.get ( av::Metadata::COMPOSER ); }

                        if ( !_metadata.get ( av::Metadata::PERFORMER ).empty() )
                        { _performer = _metadata.get ( av::Metadata::PERFORMER ); }

                        if ( !_metadata.get ( av::Metadata::COMMENT ).empty() )
                        { _comment = _metadata.get ( av::Metadata::COMMENT ); }

                        if ( !_metadata.get ( av::Metadata::YEAR ).empty() )
                        { _year = _metadata.get ( av::Metadata::YEAR ); }

                        if ( !_metadata.get ( av::Metadata::TRACK ).empty() )
                        { _track = _metadata.get ( av::Metadata::TRACK ); }

                        if ( !_metadata.get ( av::Metadata::DISC ).empty() )
                        { _disc = _metadata.get ( av::Metadata::DISC ); }

                        if ( !_metadata.get ( av::Metadata::GENRE ).empty() )
                        { _genre = _metadata.get ( av::Metadata::GENRE ); }

                        if ( !_metadata.get ( av::Metadata::PUBLISHER ).empty() )
                        { _publisher = _metadata.get ( av::Metadata::PUBLISHER ); }

                        auto codec = _format.find_codec ( av::CODEC_TYPE::AUDIO );
                        data::save( redis, data::hash ( __file[param::PATH] ), {
                            { param::PARENT, key },
                            { param::TITLE, _title },
                            { param::ALBUM, _album },
                            { param::ARTIST, _artist },
                            { param::COMPOSER, _composer },
                            { param::PERFORMER, _performer },
                            { param::COMMENT, _comment },
                            { param::YEAR, _year },
                            { param::TRACK, clean_track_number( _track ) },
                            { param::DISC, _disc },
                            { param::GENRE, _genre },
                            { param::PUBLISHER, _publisher },
                            { param::BITRATE, std::to_string ( codec->bitrate() ) },
                            { param::BPS, std::to_string ( codec->bits_per_sample() ) },
                            { param::CHANNELS, std::to_string ( codec->channels() ) },
                            { param::SAMPLERATE, std::to_string ( codec->sample_rate() ) },
                            { param::PLAYTIME, std::to_string ( _format.playtime() ) }
                        });

                        //update relation with track as score
                        data::add_types( redis, key, data::hash ( __file[param::PATH] ), data::time_millis() );
                        data::add_nodes( redis, key, data::NodeType::audio,
                                         data::hash ( __file[param::PATH] ),
                                         std::stoi( clean_track_number( _track ) ) );
                    }
                }

            } else if ( __type.first == data::NodeType::image ) {
                for ( auto& __file : __type.second ) {
                    utils::Image image_meta_ ( __file[param::PATH] );
                    data::save( redis, data::hash ( __file[param::PATH] ), {
                        { param::PARENT, key },
                        { param::CLASS, data::NodeType::str ( data::NodeType::cover ) },
                        { param::WIDTH, std::to_string ( image_meta_.width() ) },
                        { param::HEIGHT, std::to_string ( image_meta_.height() ) }
                    });
                    auto _filename = filename ( __file[param::PATH], false );
                    boost::to_lower ( _filename );

                    //store cover uri in redis map
                    _last_cover = utils::make_cover_uri ( ECoverSizes::TN, data::hash ( __file[param::PATH] ) );
                    if ( std::find (  album_cover_names.begin(),  album_cover_names.end(), _filename ) !=  album_cover_names.end() ) {
                        _cover_found = true;
                        data::save( redis, key, {{ param::THUMB, _last_cover }} );
                    }

                    //scale image
                    image_meta_.scale ( config->tmp_directory, ECoverSizes::MED, data::hash( __file[param::PATH] ) );
                    image_meta_.scale ( config->tmp_directory, ECoverSizes::TN, data::hash( __file[param::PATH] ) );

                    data::add_types( redis, key, data::hash( __file[param::PATH] ), 0 );
                    data::rem_types( redis, key, data::hash( __file[param::PATH] ) );
                    data::rem_nodes( redis, data::NodeType::image, data::hash( __file[param::PATH] ) );
                    data::add_nodes( redis, key, data::NodeType::cover, data::hash ( __file[param::PATH] ),
                            std::find (  album_cover_names.begin(),  album_cover_names.end(), _filename ) !=  album_cover_names.end() ? 1 : 2 );
                }
            } else if ( __type.first == data::NodeType::ebook ) {
                for ( auto& __file : __type.second ) {
                    //delete ebook from new books.
                    redis->command( {redis::SREM, //TODO eBooks in album are still listed in the ebook list.
                        data::make_key( key::FS, key::NEW, data::NodeType::str( data::NodeType::ebook ) ),
                        data::hash( __file[param::PATH] ) } );
                }
            } else { spdlog::get ( LOGGER )->debug ( "OTHER TYPE: {}", data::NodeType::str ( __type.first ) ); }
        }

        //when no cover is found try to take another image
        if( !_cover_found && !_last_cover.empty() )
        { data::set( redis, key, param::THUMB, _last_cover ); }

        //store folder as album
        data::save( redis, key, {
            { param::CLASS, data::NodeType::str ( data::NodeType::album ) },
            { param::NAME, _album },
            { param::CLEAN_STRING, clean_string( _album ) },
            { param::ARTIST, _artist },
            { param::PERFORMER, _performer },
            { param::COMMENT, _comment },
            { param::YEAR, _year },
            { param::GENRE, _genre }
        } );
        //create genre tag for album
        data::add_tag( redis, param::GENRE, _genre, data::NodeType::movie, key, 0 );
        //add album relation
        data::add_nodes( redis, data::NodeType::album, key, data::time_millis() );
        import_artist ( redis, key, _artist );

    } catch ( ... ) {
        spdlog::get ( LOGGER )->error ( "exception mod albums." );
    }
}

void ModAlbums::import_files ( data::redis_ptr redis, const std::string& key, std::map< data::NodeType::Enum, std::vector< data::node_t > >& files ) {
    data::children( redis, key, 0, -1, "default", "asc", "", [&redis,&key,&files](const std::string& __key ) {
        data::node_t _file = data::node ( redis, __key );
        if ( _file[param::CLASS] == data::NodeType::str ( data::NodeType::folder ) ) {
            import_files ( redis, data::hash( _file[param::PATH] ), files );
            //remove folder and relations
            //TODO the nodes to this folders still exist
            data::rem_types( redis, key, _file[param::PATH] );
            redis->command( {redis::DEL,
                data::make_key_list( data::hash( _file[param::PATH] ) ),
                data::make_key_node( data::hash( _file[param::PATH] ) ) } );

        } else { files[ data::NodeType::parse ( _file[param::CLASS] ) ].push_back ( _file ); }
    });
}

/** @brief import the artist */
void ModAlbums::import_artist ( data::redis_ptr redis, const std::string& album_key, const std::string& artist ) {

    auto _clean_string = clean_string ( artist );
    data::save( redis, data::hash ( _clean_string ), {
        { param::CLASS, data::NodeType::str ( data::NodeType::artist ) },
        { param::PARENT, album_key },
        { param::NAME, artist },
        { param::CLEAN_STRING, _clean_string }
    });
    data::add_nodes( redis, data::NodeType::artist, data::hash ( _clean_string ), data::time_millis() );
    data::add_types( redis, data::hash ( _clean_string ), album_key, data::time_millis() );
}
}//namespace mod
}//namespace cds
