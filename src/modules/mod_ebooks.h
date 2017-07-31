#ifndef MOD_EBOOKS_H
#define MOD_EBOOKS_H

#include <string>

#include "../config.h"
#include "../datastore.h"

namespace cds {
namespace mod {

class ModEbooks {
public:
    static void import( data::redox_ptr rdx, const CdsConfig& config  );
private:
    ModEbooks();
    static const std::string isbn_regex;
    static const std::string isbn_number_only_regex;
    static std::string parsePdf( const std::string & filename );
    static const std::string find_isbn ( const std::string& body );
    static const std::string remove_special_characters( const std::string& body );
};
}//namespace mod
}//namespace cds
#endif // MOD_EBOOKS_H
