#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/scanner.h"

namespace cds {
TEST( ScannerTest, remove_disc ) {
    EXPECT_EQ( "/ALBUM - 1201 - ALBUMS", Scanner::remove_disc( "/ALBUM - 1201 - ALBUMS/CD01" ) );
    EXPECT_EQ( "/ALBUM - 1201 - ALBUMS", Scanner::remove_disc( "/ALBUM - 1201 - ALBUMS/cd1" ) );
    EXPECT_EQ( "/ALBUM - 1201 - ALBUMS", Scanner::remove_disc( "/ALBUM - 1201 - ALBUMS/disc1" ) );
    EXPECT_EQ( "/ALBUM - 1201 - ALBUMS", Scanner::remove_disc( "/ALBUM - 1201 - ALBUMS/disk1" ) );
    EXPECT_EQ( "/srv/Music/Experimental/ARTIST - 1201 - ALBUM", Scanner::remove_disc( "/srv/Music/Experimental/ARTIST - 1201 - ALBUM" ) );
    EXPECT_EQ( "/Diskoholics - 1201 - ALBUMS", Scanner::remove_disc( "/Diskoholics - 1201 - ALBUMS" ) );
    EXPECT_EQ( "/Artist - 1201 - ALBUMS (234234CD)", Scanner::remove_disc( "/Artist - 1201 - ALBUMS (234234CD)" ) );
    EXPECT_EQ( "/srv/Downloads/Diskaholics Anonymous Trio - 2006 - Live In Japan Vol. 1",
               Scanner::remove_disc( "/srv/Downloads/Diskaholics Anonymous Trio - 2006 - Live In Japan Vol. 1" ) );

}
}//namespace cds
