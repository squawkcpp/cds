#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../config.h"

namespace cds {

TEST( ConfigTest, to_json ) {

const char* _result = R"json({
    "tmpDirectory": "/tmp",
    "tmdbKey": "TMDB_id",
    "amazonAccessKey": "AMAZON_ACCESS_key",
    "amazonKey": "AMAZON_key",
    "media": [
        "/dir/a",
        "/dir/b"
    ],
    "modules": [
        "a",
        "b",
        "c"
    ]
})json";

    CdsConfig config { "/tmp", "TMDB_id", "AMAZON_ACCESS_key", "AMAZON_key", { "/dir/a", "/dir/b" }, {"a", "b", "c"} };
    EXPECT_EQ( _result, json( config ) );
}
TEST( ConfigTest, from_json ) {

const char* _result = R"json({
    "tmpDirectory" : "/tmp",
    "tmdbKey" : "TMDB_id",
    "amazonAccessKey" : "AMAZON_ACCESS_key",
    "amazonKey" : "AMAZON_key",
    "media": [
        "/dir/a",
        "/dir/b"
    ]
})json";

    CdsConfig config = json( _result );
    std::vector< std::string > _medias { "/dir/a", "/dir/b" };
    EXPECT_EQ( _medias, config.media );
    EXPECT_EQ( "TMDB_id", config.tmdb_key );
    EXPECT_EQ( "AMAZON_ACCESS_key", config.amazon_access_key );
    EXPECT_EQ( "AMAZON_key", config.amazon_key );
    EXPECT_EQ( "/tmp", config.tmp_directory );
}
}//namespace cds
