#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/_utils.h"

namespace cds {
TEST(UtilsTest, clean_isbn ) {
    EXPECT_EQ( "123123123123", clean_isbn( "123-123-123-123" ) );
    EXPECT_EQ( "123123123123", clean_isbn( "123 123 123 123" ) );
    EXPECT_EQ( "X123123123123", clean_isbn( "X 123 123 123 123" ) );
    EXPECT_EQ( "9780544715301", clean_isbn( "urn:isbn:9780544715301" ) );
    EXPECT_EQ( "9780544715301", clean_isbn( "epub::ISBN 9780544715301" ) );
    EXPECT_EQ( "9780231168908", clean_isbn( "ISBN-978-0-231-16890-8" ) );
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

    EXPECT_EQ( "xconfessions vol 1 2014", remove_skip_list( trash_words, "xconfessions vol 1 2014  -rarbg" ) );
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

TEST( UtilsTest, clean_track_number ) {

    EXPECT_EQ( "1", clean_track_number( "1" ) );
    EXPECT_EQ( "1", clean_track_number( "01" ) );
    EXPECT_EQ( "1", clean_track_number( "01/13" ) );
}
TEST( UtilsTest, rank ) {

    EXPECT_EQ( 1001U, rank( "1", clean_track_number( "1" ) ) );
    EXPECT_EQ( 1001U, rank( "01", clean_track_number( "01" ) ) );
    EXPECT_EQ( 1001U, rank( "01/13", clean_track_number( "01/13" ) ) );
}

}//namespace cds
