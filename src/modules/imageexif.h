#ifndef IMAGEEXIF_H
#define IMAGEEXIF_H

#include <map>
#include <string>

namespace cds {
namespace module {
class ImageExif {
public:
    ImageExif();
    static bool load( const std::string& filename );

private:
};
}//namespace module
}//namespace cds
#endif // IMAGEEXIF_H
