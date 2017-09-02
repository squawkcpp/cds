#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/datastore.h"

namespace cds {
namespace data {

TEST(DatastoreTest, key_node ) {
    EXPECT_EQ( "fs:node_id", make_key_node( "node_id" ) );
}
TEST(DatastoreTest, key_types ) {
    EXPECT_EQ( "fs:node_id:list", make_key_list( "node_id" ) );
}
TEST(DatastoreTest, key_list ) {
    EXPECT_EQ( "fs:album:list", make_key_nodes( NodeType::album ) );
}
TEST(DatastoreTest, key_folder ) {
    EXPECT_EQ( "fs:album:parent", make_key_folders( NodeType::album ) );
}
}}//namespace data
