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
#ifndef IMAGEEXIF_H
#define IMAGEEXIF_H

#include <map>
#include <string>

#include "../datastore.h"

namespace utils {
/** @brief load exif information from image and store in node_t. */
void exif( data::node_t& node /** @param image node to process. */ );
}//namespace utils
#endif // IMAGEEXIF_H
