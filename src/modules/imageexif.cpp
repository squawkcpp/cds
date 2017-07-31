#include "imageexif.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <memory>

#include "exif.h"

namespace cds {
namespace module {

ImageExif::ImageExif() {}

bool ImageExif::load( const std::string& filename ) {

    std::ifstream file( filename, std::ios::binary | std::ios::ate );
    std::streamsize size = file.tellg();
    file.seekg( 0, std::ios::beg );

    auto buffer = std::unique_ptr< uint8_t >( new uint8_t[size] );

    if ( file.read( (char*)buffer.get(), size ) ) {

        easyexif::EXIFInfo result;
        result.parseFrom( buffer.get(), size );

        printf("Camera make       : %s\n", result.Make.c_str());
        printf("Camera model      : %s\n", result.Model.c_str());
        printf("Software          : %s\n", result.Software.c_str());
        printf("Bits per sample   : %d\n", result.BitsPerSample);
        printf("Image width       : %d\n", result.ImageWidth);
        printf("Image height      : %d\n", result.ImageHeight);
        printf("Image description : %s\n", result.ImageDescription.c_str());
        printf("Image orientation : %d\n", result.Orientation);
        printf("Image copyright   : %s\n", result.Copyright.c_str());
        printf("Image date/time   : %s\n", result.DateTime.c_str());
        printf("Original date/time: %s\n", result.DateTimeOriginal.c_str());
        printf("Digitize date/time: %s\n", result.DateTimeDigitized.c_str());
        printf("Subsecond time    : %s\n", result.SubSecTimeOriginal.c_str());
        printf("Exposure time     : 1/%d s\n", (unsigned) (1.0/result.ExposureTime));
        printf("F-stop            : f/%.1f\n", result.FNumber);
        printf("ISO speed         : %d\n", result.ISOSpeedRatings);
        printf("Subject distance  : %f m\n", result.SubjectDistance);
        printf("Exposure bias     : %f EV\n", result.ExposureBiasValue);
        printf("Flash used?       : %d\n", result.Flash);
        printf("Metering mode     : %d\n", result.MeteringMode);
        printf("Lens focal length : %f mm\n", result.FocalLength);
        printf("35mm focal length : %u mm\n", result.FocalLengthIn35mm);
        printf("GPS Latitude      : %f deg\n", result.GeoLocation.Latitude);
        printf("GPS Longitude     : %f deg\n", result.GeoLocation.Longitude);
        printf("GPS Altitude      : %f m\n", result.GeoLocation.Altitude);
    } else {
        std::cout << "error loading file." << std::endl;
        return 1;
    }
    return 0;
}
}//namespace module
}//namespace cds
