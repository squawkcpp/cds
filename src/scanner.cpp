#include "scanner.h"

#include <iostream>
#include <sstream>

#include <boost/filesystem.hpp>

#include "spdlog/spdlog.h"

#include "image.h"
#include "format.h"
#include "codec.h"

#include "_utils.h"

#include "modules/imageexif.h"
#include "modules/mod_albums.h"
#include "modules/mod_ebooks.h"
#include "modules/mod_movies.h"
#include "modules/mod_series.h"

void signalHandler( int signum ) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    exit( signum );
}

int main( int argc, char* argv[] ) {

}


namespace cds {

inline void save_folder( data::redox_ptr rdx, const std::string& name, const std::string& parent, const std::string& path ) {
    std::map< std::string, std::string > _metadata_root;
    _metadata_root[PARAM_NAME] = name;
    data::save_object( rdx, parent, NodeType::folder, path, _metadata_root );
}

Scanner::Scanner() {}
void Scanner::import_files( data::redox_ptr rdx, const CdsConfig& config ) {

    //create nodes
    save_folder( rdx, "Filesystem", "", "/" );
    save_folder( rdx, VALUE_ROOT, "/", KEY_FS );

    //create the content nodes
    for( auto& __mod : menu ) {
        save_folder( rdx, __mod[PARAM_NAME], "/", __mod[PARAM_PATH] );
    }

    magic_t _magic = magic_open( MAGIC_MIME_TYPE );
    magic_load( _magic, nullptr );
    for( auto directory : config.media ) {
        std::map< std::string, std::string > _metadata_directory;
        _metadata_directory[ PARAM_NAME ] = filename( directory, false );
        data::save_object( rdx, KEY_FS, NodeType::folder, directory, _metadata_directory );
        import_directory( rdx, _magic, directory, directory);
    }
    magic_close( _magic );

    mod::ModAlbums::import( rdx, config );
    import_images( rdx );
    mod::ModMovies::import( rdx );
    mod::ModSeries::import( rdx, config );
    mod::ModEbooks::import( rdx, config );

    rdx->publish( EVENT_SCANNER, "end" );
}
void Scanner::import_directory( data::redox_ptr rdx, magic_t& _magic, const std::string& parent_key, const std::string& path ) {
    //get files in folder
    boost::filesystem::path _fs_path ( path );
    boost::filesystem::directory_iterator end_itr;
    for ( boost::filesystem::directory_iterator itr ( _fs_path ); itr != end_itr; ++itr ) {

        const std::string item_filepath_ = path + "/" + itr->path().filename().string();

        if ( boost::filesystem::is_regular_file ( itr->status() ) ) {
            const char* _mime_type = magic_file( _magic, item_filepath_.c_str() );

            std::map< std::string, std::string > _metadata;
            _metadata[ PARAM_NAME ] = filename( item_filepath_, false );
            _metadata[ KEY_EXTENSION ] = boost::filesystem::extension( itr->path() );
            _metadata[ KEY_SIZE ] = std::to_string( boost::filesystem::file_size( itr->path() ) );
            _metadata[ KEY_MIME_TYPE ] = _mime_type;

            NodeType::Enum _type = file_meta_.parse( _mime_type, item_filepath_.c_str(), _metadata );

            data::save_object( rdx, parent_key, _type, item_filepath_.c_str(), _metadata );
            data::incr_mime( rdx, _mime_type );

        } else if ( boost::filesystem::is_directory ( itr->status() ) ) {
            std::map< std::string, std::string > _metadata;
            _metadata[ PARAM_NAME ] = filename( item_filepath_, false );
            _metadata[ PARAM_CLASS ] = VALUE_FOLDER;

            data::save_object( rdx, path, NodeType::folder, item_filepath_, _metadata );
            import_directory( rdx, _magic, item_filepath_, item_filepath_ );
        }
    }
}


void Scanner::import_images( data::redox_ptr rdx ) {
    redox::Command<std::unordered_set< std::string > >& c =
            rdx->commandSync<std::unordered_set< std::string > >( { "SMEMBERS", "fs:image" } );

    if( c.ok() ) {
        for( const std::string& __c : c.reply() ) {
            std::map< std::string, std::string > _file = data::node( rdx, __c );
            std::cout << "parse image:" << _file[PARAM_NAME] << std::endl;
            //TODO module::ImageExif::load( _file[KEY_PATH] );

            image::Image image_meta_ ( _file[PARAM_PATH] );
            rdx->command( {"HMSET",  fmt::format( "fs:{}", __c ),
                           KEY_WIDTH, std::to_string( image_meta_.width() ),
                           KEY_HEIGHT, std::to_string( image_meta_.height() ) } );

            std::string _thumb = "/tn_";
            _thumb.append( __c );
            _thumb.append( ".jpg" );
            rdx->command( {"HMSET",  fmt::format( "fs:{}", __c ), "thumb", _thumb });
            image_meta_.scale ( 768, 768, fmt::format( "{0}/med_{1}.jpg", "/var/tmp/squawk/covers" /*TODO*/, hash( _file[PARAM_PATH] ) ) );
            image_meta_.scale ( 160, 160, fmt::format( "{0}/tn_{1}.jpg", "/var/tmp/squawk/covers" /*TODO*/, hash( _file[PARAM_PATH] ) ) );
        }
    }
}
}//namespace cds
