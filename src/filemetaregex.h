#ifndef FILEMETAREGEX_H
#define FILEMETAREGEX_H

#include <map>
#include <regex>
#include <string>
#include <vector>

#include "datastore.h"

namespace cds {

struct RuleItem {
    NodeType::Enum type;
    std::regex regex;
    std::vector< std::string > names;
};

class FileMetaRegex {
public:
    FileMetaRegex() {}
    NodeType::Enum parse( const std::string& mime_type, const std::string& path, std::map< std::string, std::string >& metadata );
private:
    static std::map< std::string, std::vector< RuleItem > > rules_;
};
}//namespace cds
#endif // FILEMETAREGEX_H
