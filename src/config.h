#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"

namespace cds {
struct CdsConfig {
    std::string tmp_directory, tmdb_key;
    std::string amazon_access_key, amazon_key;
    std::vector< std::string > media;
    std::vector< std::string > modules;
};

const static char* CONFIG_TMP_DIRECTORY = "tmpDirectory";
const static char* CONFIG_TMDB_KEY = "tmdbKey";
const static char* CONFIG_AMAZON_ACCESS_KEY = "amazonAccessKey";
const static char* CONFIG_AMAZON_KEY = "amazonKey";

inline std::string json( CdsConfig& config ) {
    using namespace rapidjson;
    StringBuffer sb;
    PrettyWriter<StringBuffer> writer(sb);
    writer.StartObject();

    writer.String( CONFIG_TMP_DIRECTORY );
    writer.String( config.tmp_directory.c_str() );

    writer.String( CONFIG_TMDB_KEY );
    writer.String( config.tmdb_key.c_str() );

    writer.String( CONFIG_AMAZON_ACCESS_KEY );
    writer.String( config.amazon_access_key.c_str() );

    writer.String( CONFIG_AMAZON_KEY );
    writer.String( config.amazon_key.c_str() );

    writer.String( "media" );
    writer.StartArray();
    for( auto& __directory : config.media ) {
        writer.String( __directory.c_str() );
    }
    writer.EndArray();

    writer.String( "modules" );
    writer.StartArray();
    for( auto& __module : config.modules ) {
        writer.String( __module.c_str() );
    }
    writer.EndArray();

    writer.EndObject();
    return sb.GetString();
}
inline CdsConfig json( const std::string& config ) {
    using namespace rapidjson;
    Document document;
    document.Parse(config.c_str());

    CdsConfig _cds;
    if(document.HasMember(CONFIG_TMP_DIRECTORY) && document[CONFIG_TMP_DIRECTORY].IsString() )
    { _cds.tmp_directory = document[CONFIG_TMP_DIRECTORY].GetString(); }

    if(document.HasMember(CONFIG_TMDB_KEY) && document[CONFIG_TMDB_KEY].IsString() )
    { _cds.tmdb_key = document[CONFIG_TMDB_KEY].GetString(); }

    if(document.HasMember(CONFIG_AMAZON_ACCESS_KEY) && document[CONFIG_AMAZON_ACCESS_KEY].IsString() )
    { _cds.amazon_access_key = document[CONFIG_AMAZON_ACCESS_KEY].GetString(); }

    if(document.HasMember(CONFIG_AMAZON_KEY) && document[CONFIG_AMAZON_KEY].IsString() )
    { _cds.amazon_key = document[CONFIG_AMAZON_KEY].GetString(); }

    if(document.HasMember("media") &&
       document["media"].IsArray() ) {
        for( auto& v : document["media"].GetArray() )
        { _cds.media.push_back( v.GetString() ); }
    }

    if(document.HasMember("modules") &&
       document["modules"].IsArray() ) {
        for( auto& v : document["modules"].GetArray() )
        { _cds.modules.push_back( v.GetString() ); }
    }

    return _cds;
}
}//namespace cds
#endif // CONFIG_H
