#ifndef MOD_MOVIES_H
#define MOD_MOVIES_H

#include <string>

#include "../datastore.h"

namespace cds {
namespace mod {

class ModMovies {
public:
    static void import( data::redox_ptr rdx );

private:
    ModMovies() {}
};
}//namespace mod
}//namespace cds
#endif // MOD_MOVIES_H
