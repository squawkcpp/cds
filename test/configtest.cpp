#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/config.h"

namespace cds {

TEST( ConfigTest, to_json ) {

const char* _result = R"json({
    "listen": "0.0.0.0",
    "http-port": "80",
    "tmp-directory": "/tmp",
    "tmdb-key": "TMDB_id",
    "amazon-access-key": "AMAZON_ACCESS_key",
    "amazon-key": "AMAZON_key",
    "directory": [
        "/dir/a",
        "/dir/b"
    ]
})json";

    config_ptr config = std::make_shared< Config >();
    config->amazon_access_key = "AMAZON_ACCESS_key";
    config->amazon_key = "AMAZON_key";
    config->http_port ="80";
    config->listen_address ="0.0.0.0";
    config->media = { "/dir/a", "/dir/b" };
    config->tmdb_key = "TMDB_id";
    config->tmp_directory = "/tmp";
    EXPECT_EQ( _result, json( config ) );
}
TEST( ConfigTest, from_json ) {

const char* _result = R"json({
    "listen": "0.0.0.0",
    "http-port": "80",
    "tmp-directory": "/tmp",
    "tmdb-key": "TMDB_id",
    "amazon-access-key": "AMAZON_ACCESS_key",
    "amazon-key": "AMAZON_key",
    "directory": [
        "/dir/a",
        "/dir/b"
    ]
})json";

    auto config = json( _result );
    std::vector< std::string > _medias { "/dir/a", "/dir/b" };
    EXPECT_EQ( _medias, config->media );
    EXPECT_EQ( "TMDB_id", config->tmdb_key );
    EXPECT_EQ( "AMAZON_ACCESS_key", config->amazon_access_key );
    EXPECT_EQ( "AMAZON_key", config->amazon_key );
    EXPECT_EQ( "/tmp", config->tmp_directory );
}
}//namespace cds
