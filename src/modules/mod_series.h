#ifndef MOD_SERIES_H
#define MOD_SERIES_H

#include <string>

#include "../config.h"
#include "../datastore.h"

namespace cds {
namespace mod {

class ModSeries {
public:
    static void import( data::redox_ptr rdx, const CdsConfig& config  );

private:
    ModSeries() {}

    static std::string import( data::redox_ptr rdx, const CdsConfig& config, const std::string& serie_key, const std::string& serie );

    static std::string tmdb_get ( const std::string& api_key, const std::string& name );
    static std::string tmdb_episode ( const std::string& api_key, const std::string& serie_id, const std::string& season, const std::string& episode );
    static std::vector < std::map<std::string, std::string > > tmdb_parse( const std::string& result );
    static std::map<std::string, std::string> tmdb_parse_episode( const CdsConfig& config, const std::string& result );
    static void tmdb_fetch( const std::string& uri, const std::string& path );
};
}//namespace mod
}//namespace cds
#endif // MOD_SERIES_H
