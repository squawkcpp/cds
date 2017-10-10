#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/datastore.h"

//namespace data {

TEST(DatastoreTest, key_node ) {
    EXPECT_EQ( "fs:node_id:node", data::make_key_node( "node_id" ) );
}
TEST(DatastoreTest, key_types ) {
    EXPECT_EQ( "fs:node_id:list", data::make_key_list( "node_id" ) );
}
TEST(DatastoreTest, key_list ) {
    EXPECT_EQ( "fs:album:list", data::make_key_list( data::NodeType::album ) );
}
TEST(DatastoreTest, to_map ) {
    auto _map = data::to_map( { "key1", "value1", "key2", "value2" } );
    EXPECT_EQ( 2U, _map.size() );
    EXPECT_EQ( "value1", _map["key1"] );
    EXPECT_EQ( "value2", _map["key2"] );
}
TEST( DatastoreTest, menu ) {
    int count = 0;
    for( auto& __mod : _internal::menu ) {
        EXPECT_FALSE( __mod.empty() );
        ++count;
    }
    EXPECT_EQ( 7, count );
}
TEST( DatastoreTest, is_mod ) {
    EXPECT_TRUE( data::is_mod( param::ALBUM ) );
    EXPECT_TRUE( data::is_mod( "ebook" ) );
    EXPECT_FALSE( data::is_mod( "foo" ) );
    EXPECT_FALSE( data::is_mod( "98837641092380" ) );
}
//}//namespace data
