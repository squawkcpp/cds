#ifndef SCANNER_H
#define SCANNER_H

#include <string>
#include <vector>

#include <magic.h>
#include <redox.hpp>

#include "config.h"
#include "filemetaregex.h"

namespace cds {

class Scanner {
public:
    Scanner();
    void import_files( data::redox_ptr rdx, const CdsConfig& config );
private:
    FileMetaRegex file_meta_;
    void import_directory( data::redox_ptr rdx, magic_t& _magic, const std::string& parent_key, const std::string& path );

    void import_video( data::redox_ptr rdx );
    void import_images( data::redox_ptr rdx );
    void import_books( data::redox_ptr rdx );
};
}//namespace cds
#endif // SCANNER_H
