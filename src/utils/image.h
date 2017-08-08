/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef IMAGE_H
#define IMAGE_H

#include <string>

#include <opencv2/opencv.hpp>

#include "../_utils.h"

namespace utils {

double image_scale ( cds::ECoverSizes::Enum type, double width, double height );
std::string make_cover_uri ( const cds::ECoverSizes::Enum& type, const std::string& key );
std::string make_cover_path ( const std::string& path, const cds::ECoverSizes::Enum& type, const std::string& key );

/** @brief image manipulation utility */
class Image {
public:
    /** @brief create a new image object */
    Image( const std::string& path );

    /** @brief image width */
    int width();
    /** @brief image height */
    int height();

    /** @brief scale image */
    std::string /** @return image uri */ scale(
            const std::string& path /** @param path temporary folder to put images to */,
            const cds::ECoverSizes::Enum& type /** /@param type target the image type */,
            const std::string& key /** @param key storage key */ );
private:
    cv::Mat image_;
};
}//namespace utils
#endif // IMAGE_H
