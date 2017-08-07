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

#include "spdlog/spdlog.h"

#include "image.h"
#include "format.h"
#include "codec.h"

#include "rapidxml_ns.hpp"

#include "../_utils.h"
#include "../datastore.h"

#include "http/httpclient.h"

//TODO set album name and artist also parent (album)
//TODO store genre as tag
//TODO import images

namespace cds {
namespace mod {
void ModAlbums::import ( data::redis_ptr rdx, const config_ptr config ) {
    redox::Command< data::nodes_t >& c =
        rdx->commandSync< data::nodes_t > ( { REDIS_MEMBERS, data::make_key_folders ( NodeType::audio ) } );

    if ( c.ok() ) {
        for ( const std::string& __c : c.reply() ) {
            std::map< NodeType::Enum, std::vector< data::node_t > > _files;
            import ( rdx, __c, _files );
            std::string _artist = "", _album = "", _year = "", _track = "", _disc = "", _genre = "";

            for ( auto& __type : _files ) {
                if ( __type.first == NodeType::audio ) {
                    for ( auto& __file : __type.second ) {
                        _artist = __file[av::Metadata::name ( av::Metadata::ARTIST )];
                        _album = __file[av::Metadata::name ( av::Metadata::ALBUM )];
                        _year = __file[av::Metadata::name ( av::Metadata::YEAR )];
                        _track = __file[av::Metadata::name ( av::Metadata::TRACK )];
                        _disc = __file[av::Metadata::name ( av::Metadata::DISC )];
                        //Get the track information
                        av::Format _format ( __file[PARAM_PATH] );

                        if ( !!_format ) {
                            spdlog::get ( LOGGER )->warn ( "Can not open audio file:{} ({})",
                                                           _format.errc().message(),
                                                           __file[PARAM_PATH] );
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

                            //load musicbrainz information

                            auto codec = _format.find_codec ( av::CODEC_TYPE::AUDIO );
                            rdx->command ( {REDIS_SET,  data::make_key_node ( hash ( __file[PARAM_PATH] ) ),
                                            PARAM_PARENT, __c,
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
                            //TODO update relation with track as score
                        }
                    }

                } else if ( __type.first == NodeType::image ) {
                    for ( auto& __file : __type.second ) {
                        image::Image image_meta_ ( __file[PARAM_PATH] );
                        rdx->command ( {REDIS_SET,  data::make_key_node ( hash ( __file[PARAM_PATH] ) ),
                                        PARAM_PARENT, __c,
                                        PARAM_CLASS, NodeType::str ( NodeType::cover ),
                                        KEY_WIDTH, std::to_string ( image_meta_.width() ),
                                        KEY_HEIGHT, std::to_string ( image_meta_.height() )
                                       } );
                        auto _filename = filename ( __file[PARAM_PATH], false );
                        boost::to_lower ( _filename );

                        if ( std::find (  album_cover_names.begin(),  album_cover_names.end(), _filename ) !=  album_cover_names.end() ) {
                            rdx->command ( {REDIS_SET, data::make_key_node ( __c ), PARAM_THUMB,
                                            make_cover_uri ( ECoverSizes::TN, hash ( __file[PARAM_PATH] ) )
                                           } );
                        }



                        //scale image
                        //TODO                        float _med_scale = image_scale( IMAGE_MED_SIZE, image_meta_.width(), image_meta_.height() );
                        //                        float _tn_scale = image_scale( IMAGE_TN_SIZE, image_meta_.width(), image_meta_.height() );
                        //                        image_meta_.scale ( ((float)image_meta_.width()/_med_scale), ((float)image_meta_.height()/_med_scale),
                        //                                            make_cover_path( config->tmp_directory, ECoverSizes::MED, hash( __file[PARAM_PATH] ) ) );
                        //                        image_meta_.scale ( ((float)image_meta_.width()/_tn_scale), ((float)image_meta_.height()/_tn_scale),
                        //                                            make_cover_path( config->tmp_directory, ECoverSizes::TN, hash( __file[PARAM_PATH] ) ) );

                        data::add_types( rdx, NodeType::cover, __c, hash( __file[PARAM_PATH] ) );
                        data::rem_types( rdx, NodeType::image, __c, hash( __file[PARAM_PATH] ) );
                        data::rem_nodes( rdx, NodeType::image, hash( __file[PARAM_PATH] ) );
                        rdx->command( {REDIS_REM,  data::make_key_types( __c , NodeType::image ), hash( __file[PARAM_PATH] ) } );
                    }
                } else { spdlog::get ( LOGGER )->debug ( "OTHER TYPE: {}", NodeType::str ( __type.first ) ); }

                //TODO when no cover is found try to take another image


                rdx->command ( {REDIS_SET,  data::make_key_node ( __c ),
                                PARAM_CLASS, NodeType::str ( NodeType::album ),
                                av::Metadata::name ( av::Metadata::ARTIST ), _artist,
                                av::Metadata::name ( av::Metadata::ALBUM ), _album,
                                av::Metadata::name ( av::Metadata::YEAR ), _year,
                                av::Metadata::name ( av::Metadata::TRACK ), _track,
                                av::Metadata::name ( av::Metadata::DISC ), _disc,
                                av::Metadata::name ( av::Metadata::GENRE ), _genre
                               } );
                //TODO ordered set
                rdx->command ( {REDIS_ADD,  data::make_key_nodes ( NodeType::album ), __c } );
                import ( rdx, __c, _artist );
            }
        }
    }
}

void ModAlbums::import ( data::redis_ptr rdx, const std::string& key, std::map< NodeType::Enum, std::vector< data::node_t > >& files ) {
    std::vector< std::string > _command;
    _command.push_back ( REDIS_UNION );

    for ( auto& __type : NodeTypes )
    {  _command.push_back ( data::make_key_types ( key, __type ) ); }

    redox::Command< data::nodes_t >& c =
        rdx->commandSync< data::nodes_t > ( _command );

    if ( c.ok() ) {
        for ( const std::string& __key : c.reply() ) {
            data::node_t _file = data::node ( rdx, __key );

            if ( _file[PARAM_CLASS] == NodeType::str ( NodeType::folder ) ) {
                import ( rdx, _file[PARAM_PATH], files );
            } else { files[ NodeType::parse ( _file[PARAM_CLASS] ) ].push_back ( _file ); }
        }
    }
}

/** @brief import the artist */
void ModAlbums::import ( data::redis_ptr rdx, const std::string& album_key, const std::string& artist ) {
    auto _clean_string = clean_string ( artist );
    std::vector< std::string > _commands = {
        REDIS_SET,  data::make_key_node ( hash ( _clean_string ) ),
        PARAM_CLASS, NodeType::str ( NodeType::artist ),
        av::Metadata::name ( av::Metadata::ARTIST ), artist
    };
    //get the artist cover
    //TODO result is multipart encoded
    //    std::string _mbid = mbid_parse( mbid_get( artist ) );
    //    if( !_mbid.empty() ) {
    //        auto _metadata = get_artist_metadata( artist_meta_get( _mbid ) );
    //        for( auto __m_item : _metadata ) {
    //            //TODO if( __m_item.first == "image" ) {
    //            //} else {
    //                _commands.push_back( __m_item.first );
    //                _commands.push_back( __m_item.second );
    //            //}
    //        }
    //    }
    rdx->command ( _commands );
    rdx->command ( { REDIS_ADD, data::make_key_types ( hash ( _clean_string ), NodeType::album ), hash ( album_key ) } );
    rdx->command ( { REDIS_ADD, data::make_key_nodes ( NodeType::artist ), hash ( _clean_string ) } );
}

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
