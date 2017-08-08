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
#include "filemetaregex.h"

#include <iostream>

#include "spdlog/spdlog.h"

#include <boost/algorithm/string.hpp>

#include "format.h"

namespace cds {

#define REG_PRE "^(/.*)*/"
#define REG_TRACK "([A-Z]?[0-9]+)"
#define REG_ARTIST "(.*)"
#define REG_ALBUM "(.*)"
#define REG_YEAR "([0-9]+)"
#define REG_TITLE "(.*)"
#define _ " ?- ?"
#define _TIT " ?\\.?-? ?"

NodeType::Enum FileMetaRegex::parse ( const std::string& mime_type, const std::string& path, data::node_t& metadata ) {
    if ( mime_type == "text/plain" ) {
        return NodeType::file;
    }

    for ( auto& _mime : rules_ ) {
        if ( mime_type.find ( _mime.first ) == 0 ) {
            std::smatch base_match;
            std::string _search_path = remove_skip_list ( trash_words, remove_extension ( path ) );

            for ( auto& _rule : _mime.second ) {
                if ( std::regex_match ( _search_path, base_match, _rule.regex ) ) {
                    size_t j = 0;

                    for ( size_t i = base_match.size() - _rule.names.size(); i < base_match.size(); ++i ) {
                        std::string s = base_match[i];
                        metadata[_rule.names.at ( j )] = boost::trim_copy ( s );
                        ++j;
                    }
                    return _rule.type;
                }
            }
            spdlog::get ( LOGGER )->info ( "rule not found for file:{0}", path );
        }
    }

    if ( mime_type.find ( "audio/" ) == 0 ) {
        return NodeType::audio;
    }

    if ( mime_type.find ( "video/" ) == 0 ) {
        return NodeType::movie;
    }

    if ( mime_type.find ( "image/" ) == 0 ) {
        return NodeType::image;
    }

    if ( mime_type == "application/pdf" ) {
        return NodeType::ebook;
    }

    if ( mime_type.find ( "application/octet-stream" ) == 0 ) {
        return NodeType::file;
    }

    spdlog::get ( LOGGER )->info ( "rule not found for mime_type:{0}", mime_type );
    return NodeType::file;
}

std::map< std::string, std::vector< RuleItem > > FileMetaRegex::rules_ = {
    {
        "audio/", {
            {
                NodeType::audio,
                std::regex ( REG_PRE REG_ARTIST _ REG_YEAR _ REG_ALBUM "/" REG_TRACK _TIT REG_TITLE "$" ),
                { PARAM_ARTIST, PARAM_YEAR, PARAM_ALBUM, "track", PARAM_NAME }
            },
            {
                NodeType::audio,
                std::regex ( REG_PRE REG_ARTIST _ REG_ALBUM "\\(" REG_YEAR "\\) */" REG_TRACK _TIT REG_TITLE "$" ),
                { PARAM_ARTIST, PARAM_ALBUM, PARAM_YEAR, "track", PARAM_NAME }
            },
            {
                NodeType::audio,
                std::regex ( REG_PRE REG_YEAR _ REG_ARTIST _ REG_ALBUM "/" REG_TRACK _TIT REG_TITLE "$" ),
                { PARAM_YEAR, PARAM_ARTIST, PARAM_ALBUM, "track", PARAM_NAME }
            },
            {
                NodeType::audio,
                std::regex ( REG_PRE REG_ARTIST _ REG_ALBUM " */" REG_TRACK _TIT REG_TITLE "$" ),
                { PARAM_ARTIST, PARAM_ALBUM, "track", PARAM_NAME }
            },
        }
    },
    {
        "video/", {
            //SERIE S00E00 TITLE
            {
                NodeType::episode,
                std::regex ( "^(/.*)*/(.*) ?S([0-9]*)E([0-9]*)(.*)$" ),
                { PARAM_SERIE, PARAM_SEASON, PARAM_EPISODE, PARAM_NAME }
            },

            // /path/SERIE S00E00
            {
                NodeType::episode,
                std::regex ( "^(/.*)*/(.*) ?[S|s]([0-9]*)[E|e]([0-9]*)$" ),
                { PARAM_SERIE, PARAM_SEASON, PARAM_EPISODE }
            },

            //TITLE YEAR
            {
                NodeType::movie,
                std::regex ( "^(/.*)*/(.*) ([0-9]*)$" ),
                { PARAM_NAME, PARAM_YEAR }
            },

            //TITLE
            {
                NodeType::movie,
                std::regex ( "^(/.*)*/(.*)$" ),
                { PARAM_NAME }
            },

        }
    }
    /*,
    { "image/", { std::regex(), { "a", "b", "c" } } } */
};
}//namespace cds
