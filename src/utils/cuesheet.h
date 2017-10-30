#ifndef CUESHEET_H
#define CUESHEET_H

#include <string>

namespace utils {

class CueSheet {
public:
    static CueSheet parse( std::string cue_sheet );
private:
    CueSheet();
};
}//namespace utils
#endif // CUESHEET_H
