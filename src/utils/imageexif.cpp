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
#include "imageexif.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "spdlog/spdlog.h"

#include "exif.h"

#include "../_utils.h"

namespace utils {
void exif( cds::data::node_t& node /** @param image node to process. */ ) {

    std::ifstream file( node[cds::PARAM_PATH], std::ios::binary | std::ios::ate );
    std::streamsize size = file.tellg();
    file.seekg( 0, std::ios::beg );

    auto buffer = std::unique_ptr< uint8_t >( new uint8_t[size] );

    if ( file.read( (char*)buffer.get(), size ) ) {

        easyexif::EXIFInfo result;
        result.parseFrom( buffer.get(), size );

        node["Make"] = result.Make.c_str();
        node["Model"] = result.Model.c_str();
        node["Software"] = result.Software.c_str();
        node["BitsPerSample"] = std::to_string( result.BitsPerSample );
        node["width"] = std::to_string( result.ImageWidth );
        node["height"] = std::to_string( result.ImageHeight );
        node["description"] = result.ImageDescription.c_str();
        node["Orientation"] = std::to_string( result.Orientation );
        node["date"] = result.DateTime.c_str();
        node["DateTimeOriginal"] = result.DateTimeOriginal.c_str();
        node["SubSecTimeOriginal"] = result.SubSecTimeOriginal.c_str();
        node["ExposureTime"] = std::to_string( result.ExposureTime );
        node["FNumber"] = std::to_string( result.FNumber );
        node["ISOSpeedRatings"] = std::to_string( result.ISOSpeedRatings );
        node["SubjectDistance"] = std::to_string( result.SubjectDistance );
        node["ExposureBiasValue"] = std::to_string( result.ExposureBiasValue );
        node["Flash"] = std::to_string( result.Flash );
        node["MeteringMode"] = std::to_string( result.MeteringMode );
        node["FocalLength"] = std::to_string( result.FocalLength );
        node["FocalLengthIn35mm"] = std::to_string( result.FocalLengthIn35mm );
        node["GeoLocation.Latitude"] = std::to_string( result.GeoLocation.Latitude );
        node["GeoLocation.Longitude"] = std::to_string( result.GeoLocation.Longitude );
        node["GeoLocation.Altitude"] = std::to_string( result.GeoLocation.Altitude );

    } else {
        spdlog::get ( cds::LOGGER )->warn ( "error load exif: {}", node[cds::PARAM_PATH] );
    }
}
}//namespace utils
