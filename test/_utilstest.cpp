#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/_utils.h"

namespace cds {

TEST(UtilsTest, image_scale_factor ) {
    EXPECT_EQ( .5, image_scale( ECoverSizes::TN, 320, 320 ) );
    EXPECT_EQ( .5, image_scale( ECoverSizes::TN, 300, 320 ) );
    EXPECT_EQ( .5, image_scale( ECoverSizes::TN, 320, 300 ) );
}

TEST(UtilsTest, image_scale_result ) {
    float _res = image_scale( ECoverSizes::TN, 320, 320 );
    EXPECT_EQ( 160, (float)320*_res );
}

TEST(UtilsTest, clean_isbn ) {
    EXPECT_EQ( "123123123123", clean_isbn( "123-123-123-123" ) );
    EXPECT_EQ( "123123123123", clean_isbn( "123 123 123 123" ) );
    EXPECT_EQ( "X123123123123", clean_isbn( "X 123 123 123 123" ) );
    EXPECT_EQ( "9780544715301", clean_isbn( "urn:isbn:9780544715301" ) );
    EXPECT_EQ( "9780544715301", clean_isbn( "epub::ISBN 9780544715301" ) );
    EXPECT_EQ( "9780231168908", clean_isbn( "ISBN-978-0-231-16890-8" ) );
}

TEST( UtilsTest, menu ) {
    int count = 0;
    for( auto& __mod : menu ) {
        EXPECT_FALSE( __mod.empty() );
        ++count;
    }
    EXPECT_EQ( 7, count );
}

TEST( UtilsTest, is_mod ) {
    EXPECT_TRUE( is_mod( PARAM_ALBUM ) );
    EXPECT_TRUE( is_mod( "ebook" ) );
    EXPECT_FALSE( is_mod( "foo" ) );
    EXPECT_FALSE( is_mod( "98837641092380" ) );
}

TEST( UtilsTest, mod_key ) {
    EXPECT_EQ( "fs:album", mod_key( PARAM_ALBUM ) );
    EXPECT_EQ( "fs:ebook", mod_key( "ebook" ) );
}

TEST( UtilsTest, to_map ) {
    auto _map = to_map( { "key1", "value1", "key2", "value2" } );
    EXPECT_EQ( 2U, _map.size() );
    EXPECT_EQ( "value1", _map["key1"] );
    EXPECT_EQ( "value2", _map["key2"] );
}

TEST( UtilsTest, clean_string ) {
    EXPECT_EQ( "artist - 2000 - album/01 - track.mp3", clean_string( "ARTIST - 2000 - ALBUM/01 - TRACK.mp3" ) );
    EXPECT_EQ( "artist - 2000 - album/01 - track.mp3", clean_string( "The ARTIST - 2000 - the ALBUM/01 - The TRACK.mp3" ) );
    EXPECT_EQ( "artist - 2000 - album/01 - track.mp3", clean_string( "The ARTIST - 2000 - Der ALBUM/01 - Das TRACK.mp3" ) );
}
TEST( UtilsTest, remove_extension ) {
    EXPECT_EQ( "ARTIST - 2000 - ALBUM/01 - TRACK", remove_extension( "ARTIST - 2000 - ALBUM/01 - TRACK.mp3" ) );
}
TEST( UtilsTest, filename ) {
    EXPECT_EQ( "01 - track", filename( "ARTIST - 2000 - ALBUM/01 - track", true ) );
    EXPECT_EQ( "01 - track.mp3", filename( "The ARTIST - 2000 - the ALBUM/01 - track.mp3", true ) );
    EXPECT_EQ( "01 - track", filename( "The ARTIST - 2000 - Der ALBUM/01 - track.mp3", false ) );
    EXPECT_EQ( "02 - The Infernal Machine",
        filename( "/srv/Downloads/John Zorn - The Garden of Earthly Delights (2017)/02 - The Infernal Machine.mp3", false ) );

}
TEST( UtilsTest, path ) {
    EXPECT_EQ( "/foo/bar/ARTIST - 2000 - ALBUM", path( "/foo/bar/ARTIST - 2000 - ALBUM/01 - track" ) );
}
TEST( UtilsTest, skip_words ) {
    EXPECT_EQ( "ALBUM - 1201 - ALBUMS", remove_skip_list( trash_words, "ALBUM - 1201 - ALBUMS [24bit]" ) );
    EXPECT_EQ( "Teho - Jupiter  Traum Schallplatten  -2017/01 - Teho - Jupiter (original mix)",
               remove_skip_list( trash_words, "Teho_-_Jupiter__Traum_Schallplatten__FLAC-2017/01 - Teho - Jupiter (original mix)" ) );
    EXPECT_EQ( "/foo/bar/ARTIST - ALBUM (2017) /01 - TITLE",
               remove_skip_list( trash_words, "/foo/bar/ARTIST - ALBUM (2017) [96-24]/01 - TITLE" ) );
    EXPECT_EQ( "Modern Family S08E21 Alone Time     /Modern Family S08E21 Alone Time",
               remove_skip_list( trash_words, "Modern.Family.S08E21.Alone.Time.720p.AMZN.WEBRip.DD5.1.x264-NTb[rarbg]/Modern.Family.S08E21.Alone.Time.720p.AMZN.WEBRip.DD5.1.x264-NTb" ) );
    EXPECT_EQ( "The Simpsons S28E20   /The Simpsons S28E20",
               remove_skip_list( trash_words, "The.Simpsons.S28E20.720p.HDTV.x264-AVS[rarbg]/The.Simpsons.S28E20.720p.HDTV.x264-AVS" ) );
}
}//namespace cds
