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
}
}//namespace cds