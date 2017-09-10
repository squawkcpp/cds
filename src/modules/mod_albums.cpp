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
void ModAlbums::import ( data::redis_ptr rdx, const config_ptr config ) {
    data::new_items( rdx, data::NodeType::audio, [rdx,config]( const std::string& key ) {
            std::map< data::NodeType::Enum, std::vector< data::node_t > > _files;
            import ( rdx, key, _files );
            std::string _artist = "", _album = "", _year = "", _track = "", _disc = "", _genre = "";
            bool _cover_found = false;
            std::string _last_cover = "";

            for ( auto& __type : _files ) {
                if ( __type.first == data::NodeType::audio ) {
                    for ( auto& __file : __type.second ) {
                        _artist = __file[av::Metadata::name ( av::Metadata::ARTIST )];
                        _album = __file[av::Metadata::name ( av::Metadata::ALBUM )];
                        _year = __file[av::Metadata::name ( av::Metadata::YEAR )];
                        _track = __file[av::Metadata::name ( av::Metadata::TRACK )];
                        _disc = __file[av::Metadata::name ( av::Metadata::DISC )];
                        //Get the track information
                        av::Format _format ( __file[data::KEY_PATH] );

                        if ( !!_format ) {
                            spdlog::get ( LOGGER )->warn ( "Can not open audio file:{} ({})",
                                                           _format.errc().message(),
                                                           __file[data::KEY_PATH] );
                        } else {
                            av::Metadata _metadata = _format.metadata();

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

                            //TODO load musicbrainz information
                            auto codec = _format.find_codec ( av::CODEC_TYPE::AUDIO );
                            rdx->command ( {data::REDIS_HMSET,  data::make_key_node ( data::hash ( __file[data::KEY_PATH] ) ),
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
                            rdx->command ( {data::REDIS_ZADD, data::make_key_list ( key ), _track, data::hash ( __file[data::KEY_PATH] ) } );
                        }
                    }

                } else if ( __type.first == data::NodeType::image ) {
                    for ( auto& __file : __type.second ) {
                        utils::Image image_meta_ ( __file[data::KEY_PATH] );
                        rdx->command ( {data::REDIS_HMSET,  data::make_key_node ( data::hash ( __file[data::KEY_PATH] ) ),
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
                            rdx->command ( { data::REDIS_HMSET, data::make_key_node ( key ), PARAM_THUMB, _last_cover } );
                        }

                        //scale image
                        image_meta_.scale ( config->tmp_directory, ECoverSizes::MED, data::hash( __file[data::KEY_PATH] ) );
                        image_meta_.scale ( config->tmp_directory, ECoverSizes::TN, data::hash( __file[data::KEY_PATH] ) );

                        data::add_types( rdx, key, data::hash( __file[data::KEY_PATH] ), 0 /*TODO*/ );
                        data::rem_types( rdx, data::NodeType::image, key, data::hash( __file[data::KEY_PATH] ) );
                        data::rem_nodes( rdx, data::NodeType::image, data::hash( __file[data::KEY_PATH] ) );
                    }
                } else { spdlog::get ( LOGGER )->debug ( "OTHER TYPE: {}", data::NodeType::str ( __type.first ) ); }
            }

            //when no cover is found try to take another image
            if( !_cover_found ) {
                std::cout << "found no cover, last_cover = " << _last_cover << std::endl;
                if( !_last_cover.empty() ) {
                    rdx->command ( { data::REDIS_HMSET, data::make_key_node ( key ), PARAM_THUMB, _last_cover } );
                }
            }

            //store folder as album
            rdx->command ( {data::REDIS_HMSET,  data::make_key_node ( key ),
                            data::KEY_CLASS, data::NodeType::str ( data::NodeType::album ),
                            av::Metadata::name ( av::Metadata::ARTIST ), _artist,
                            av::Metadata::name ( av::Metadata::ALBUM ), _album,
                            av::Metadata::name ( av::Metadata::YEAR ), _year,
                            av::Metadata::name ( av::Metadata::GENRE ), _genre
            } );

            //add album with timestamp as score
            auto _last_write_time = boost::filesystem::last_write_time( data::get( rdx, key, data::KEY_PATH ) );
            rdx->command ( {data::REDIS_ZADD, data::make_key_list ( data::NodeType::album ), std::to_string( _last_write_time ), key } );
            import ( rdx, key, _artist );
        });
}

void ModAlbums::import ( data::redis_ptr rdx, const std::string& key, std::map< data::NodeType::Enum, std::vector< data::node_t > >& files ) {

    auto& c = rdx->commandSync< data::nodes_t > (
        { data::REDIS_ZRANGE, data::make_key_list ( key ), "0", "-1" } );

    if ( c.ok() ) {
        for ( const std::string& __key : c.reply() ) {
            data::node_t _file = data::node ( rdx, __key );

            if ( _file[data::KEY_CLASS] == data::NodeType::str ( data::NodeType::folder ) ) {
                import ( rdx, _file[data::KEY_PATH], files );
            } else { files[ data::NodeType::parse ( _file[data::KEY_CLASS] ) ].push_back ( _file ); }
        }
    }
}

/** @brief import the artist */
void ModAlbums::import ( data::redis_ptr rdx, const std::string& album_key, const std::string& artist ) {

    auto _clean_string = clean_string ( artist );
    rdx->command ( { data::REDIS_HMSET,  data::make_key_node ( data::hash ( _clean_string ) ),
                     data::KEY_CLASS, data::NodeType::str ( data::NodeType::artist ),
                     av::Metadata::name ( av::Metadata::ARTIST ), artist } );

    data::add_types( rdx, _clean_string, album_key, 0 );
//TODO    rdx->command ( {REDIS_ZADD, data::make_key_nodes ( data::NodeType::artist ), "0" /* TODO/*/, hash ( _clean_string ) } );
}

//TODO remove, is unused
std::string ModAlbums::mbid_get ( const std::string& name ) {
    usleep ( 20000000 ); //TODO
    http::HttpClient< http::Http>  client_ ( "musicbrainz.org", "http" ); //set user agent: https://musicbrainz.org/doc/XML_Web_Service/Rate_Limiting
    http::Request request_ ( fmt::format ( "/ws/2/artist/?query=artist:{}", http::utils::UrlEscape::urlEncode ( name ) ) );
    request_.parameter ( http::header::USER_AGENT, "SquawkMediaServer/1.0.0 ( squawkcpp@gmail.com )" );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "MusicBrainz RESULT: {}", _sstream.str() );
    return _sstream.str();
}

std::string ModAlbums::artist_meta_get ( const std::string& mbid ) {
    usleep ( 20000000 ); //TODO
    http::HttpClient< http::Http>  client_ ( "musicbrainz.org", "http" );
    http::Request request_ ( fmt::format ( "/ws/2/artist/{}?inc=url-rels", mbid ) );
    request_.parameter ( http::header::USER_AGENT, "SquawkMediaServer/1.0.0 ( squawkcpp@gmail.com )" );
    std::stringstream _sstream;
    client_.get ( request_, _sstream );
    SPDLOG_TRACE ( spdlog::get ( LOGGER ), "MusicBrainz RESULT: {}", _sstream.str() );
    return _sstream.str();
}

std::string ModAlbums::mbid_parse ( const std::string& mb_artist ) {
    rapidxml_ns::xml_document<> doc;
    doc.parse<0> ( const_cast< char* > ( mb_artist.c_str() ) );
    auto root_node = doc.first_node ( "metadata" );

    for ( rapidxml_ns::xml_node<>* __r_sieblings = root_node->first_node(); __r_sieblings; __r_sieblings = __r_sieblings->next_sibling() ) {
        if ( strcmp ( __r_sieblings->name(), "artist-list" ) == 0 ) {
            for ( rapidxml_ns::xml_node<>* __artist_node = __r_sieblings->first_node(); __artist_node; __artist_node = __artist_node->next_sibling() ) {
                if ( strcmp ( __artist_node->name(), "artist" ) == 0 ) {
                    std::string _mbid = "";
                    int _score = 0;

                    for ( rapidxml_ns::xml_attribute<>* __attr = __artist_node->first_attribute();
                            __attr; __attr = __attr->next_attribute() ) {
                        if ( strcmp ( __attr->local_name(), "id" ) == 0 ) {
                            _mbid = __attr->value();
                        } else if ( strcmp ( __attr->local_name(), "score" ) == 0 ) {
                            _score = std::atoi ( __attr->value() );
                        }
                    }//arist attrs

                    if ( _score > 95 ) {
                        return _mbid;
                    }//match
                }//if artist
            }//artists
        }//if artist list
    }//root nodes

    return "";
}
std::map<std::string, std::string> ModAlbums::get_artist_metadata ( const std::string& metadata ) {
    std::map<std::string, std::string> _relations;
    rapidxml_ns::xml_document<> doc;
    doc.parse<0> ( const_cast< char* > ( metadata.c_str() ) );
    auto root_node = doc.first_node ( "metadata" );

    for ( rapidxml_ns::xml_node<>* __r_sieblings = root_node->first_node(); __r_sieblings; __r_sieblings = __r_sieblings->next_sibling() ) {
        if ( strcmp ( __r_sieblings->name(), "artist" ) == 0 ) {
            for ( rapidxml_ns::xml_node<>* __artist_node = __r_sieblings->first_node(); __artist_node; __artist_node = __artist_node->next_sibling() ) {
                if ( strcmp ( __artist_node->name(), "relation-list" ) == 0 ) {
                    for ( rapidxml_ns::xml_node<>* __relation_item = __artist_node->first_node(); __relation_item; __relation_item = __relation_item->next_sibling() ) {
                        if ( strcmp ( __relation_item->name(), "relation" ) == 0 ) {
                            for ( rapidxml_ns::xml_attribute<>* __attr = __relation_item->first_attribute();
                                    __attr; __attr = __attr->next_attribute() ) {
                                if ( strcmp ( __attr->local_name(), "type" ) == 0 ) {
                                    if ( std::find ( metadata_relations.begin(),  metadata_relations.end(), __attr->value() ) !=  metadata_relations.end() ) {
                                        for ( rapidxml_ns::xml_node<>* __relation_target = __relation_item->first_node(); __relation_target; __relation_target = __relation_target->next_sibling() ) {
                                            if ( strcmp ( __relation_target->name(), "target" ) == 0 ) {
                                                _relations[__attr->value()] = __relation_target->value();
                                            }
                                        }//search relation
                                    }//is selected target
                                }//is type
                            }//search attributes
                        }//is relation
                    }//search relation
                }//is relation list
            }//search relation list
        }//is artist
    }//root_nodes

    return _relations;
}
}//namespace mod
}//namespace cds
