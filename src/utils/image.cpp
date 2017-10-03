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
#include "image.h"

namespace utils {

double image_scale ( cds::ECoverSizes::Enum type, double width, double height ) {
    if ( width >= height ) {
        return cds::ECoverSizes::size( type ) / width;
    } else { return cds::ECoverSizes::size( type ) / height; }
}

std::string make_cover_uri ( const cds::ECoverSizes::Enum& type, const std::string& key ) {
    return fmt::format ( "/img/{}_{}.jpg", cds::ECoverSizes::str ( type ), key );
}

std::string make_cover_path ( const std::string& path, const cds::ECoverSizes::Enum& type, const std::string& key ) {
    return fmt::format ( "{0}/{1}_{2}.jpg", path, cds::ECoverSizes::str ( type ), key );
}

Image::Image( const std::string& path ) : image_( cv::imread ( path, CV_LOAD_IMAGE_COLOR ) ) {}

int Image::width()
{ return image_.cols; }
int Image::height()
{ return image_.rows; }

std::string /** @return image uri */ Image::scale(
        const std::string& path /** @param path temporary folder to put images to */,
        const cds::ECoverSizes::Enum& type /** /@param type target the image type */,
        const std::string& key /** @param key storage key */ ) {

    try {
        auto _cover_path = make_cover_path( path, type, key );

        double _width = image_.cols;
        double _height = image_.rows;

        double _scale = image_scale( type, _width, _height );
        double _target_width = _width * _scale;
        double _target_height = _height * _scale;

        cv::Mat _target_image;
        auto _s = cv::Size( static_cast< int >( _target_width ), static_cast< int >( _target_height ) );
        cv::resize( image_, _target_image, _s );
        cv::imwrite( _cover_path, _target_image );
        return make_cover_uri( type, key );

    } catch( ... ) {
        spdlog::get ( cds::LOGGER )->warn ( "error opening image (key:{0})", key );
        return "";
    }
}
}//namespace utils
