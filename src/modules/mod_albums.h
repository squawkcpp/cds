#ifndef MOD_ALBUMS_H
#define MOD_ALBUMS_H

#include <string>

#include "../config.h"
#include "../datastore.h"

#include "gtest/gtest_prod.h"

namespace cds {
namespace mod {

class ModAlbums {
public:
    static void import( data::redox_ptr rdx, const CdsConfig& config );
private:
    static void import( data::redox_ptr rdx,
                        const std::string& key, std::map< NodeType::Enum, std::vector< std::map< std::string, std::string > > >& files );
    static void import( data::redox_ptr rdx,
                        const std::string& album_key, const std::string& artist );

    FRIEND_TEST(MusicbrainzTest, parse_artist_mbid);
    static std::string mbid_parse( const std::string& artist );
    FRIEND_TEST(MusicbrainzTest, parse_artist_metadata);
    static std::map<std::string, std::string> get_artist_metadata( const std::string& metadata );
    static std::string artist_meta_get ( const std::string& mbid );
    static std::string mbid_get ( const std::string& name );
    ModAlbums() {}
};
}//namespace mod
}//namespace cds
#endif // MOD_ALBUMS_H
