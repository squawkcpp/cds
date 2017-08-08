#include <string>
#include <map>

#include <gtest/gtest.h>

#include "../src/utils/image.h"

namespace utils {

TEST(ImageTest, image_scale_factor ) {
    EXPECT_EQ( .5, image_scale( cds::ECoverSizes::TN, 320, 320 ) );
    EXPECT_EQ( .5, image_scale( cds::ECoverSizes::TN, 300, 320 ) );
    EXPECT_EQ( .5, image_scale( cds::ECoverSizes::TN, 320, 300 ) );
}

TEST(ImageTest, image_scale_result ) {
    float _res = image_scale( cds::ECoverSizes::TN, 320, 320 );
    EXPECT_EQ( 160, (float)320*_res );
}
}//namespace cds
