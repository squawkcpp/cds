/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <string>
#include <map>

#include <gtest/gtest.h>
#include "spdlog/spdlog.h"

#include "../src/filemetaregex.h"

namespace cds {

static auto console = spdlog::stdout_color_mt(LOGGER);

class FileMetaRegexTest : public ::testing::Test {
public:
    FileMetaRegexTest( ) {}
protected:
  virtual void SetUp() {}
};

TEST_F( FileMetaRegexTest, parse_path ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/foo/bar/ARTIST - 2000 - ALBUM/01 - TRACK.flac", _metadata ) );
    for( auto& __v : _metadata ) {
        std::cout << "\t" << __v.first << "=" << __v.second << std::endl;
    }
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "ARTIST", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "2000", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "ALBUM", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "01", _metadata["track"] );
    EXPECT_EQ( "TRACK", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_mime_fallback ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/srv/Downloads/SepulturaRoots/Roots Bloody Roots.flac", _metadata ) );
    EXPECT_EQ( cds::NodeType::image, cds::FileMetaRegex::parse( "image/jpeg", "/srv/Downloads/SepulturaRoots/Roots Bloody Roots.jpg", _metadata ) );
    EXPECT_EQ( cds::NodeType::movie, cds::FileMetaRegex::parse( "video/x-matroska", "/srv/Downloads/SepulturaRoots/Roots Bloody Roots.mkv", _metadata ) );
}
TEST_F( FileMetaRegexTest, parse_rule01 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/mpeg", "/srv/Downloads/John Zorn - The Garden of Earthly Delights (2017)/07 - Mirror Image.mp3", _metadata ) );
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "John Zorn", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "2017", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "The Garden of Earthly Delights", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "07", _metadata["track"] );
    EXPECT_EQ( "Mirror Image", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule02 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::episode, cds::FileMetaRegex::parse( "video/x-matroska", "/srv/Movies/The.Simpsons.S28E20.720p.HDTV.x264-AVS[rarbg]/The.Simpsons.S28E20.720p.HDTV.x264-AVS.mkv", _metadata ) );
    EXPECT_EQ( 4u, _metadata.size() );
    EXPECT_EQ( "", _metadata[PARAM_NAME] );
    EXPECT_EQ( "The Simpsons", _metadata[PARAM_SERIE] );
    EXPECT_EQ( "28", _metadata[PARAM_SEASON] );
    EXPECT_EQ( "20", _metadata[PARAM_EPISODE] );
}
TEST_F( FileMetaRegexTest, parse_rule03 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::episode, cds::FileMetaRegex::parse( "video/x-matroska", "/srv/Movies/The.Simpsons.S28E20.Some Title 720p.HDTV.x264-AVS[rarbg]/The.Simpsons.S28E20.Some.Title720p.HDTV.x264-AVS.mkv", _metadata ) );
    EXPECT_EQ( 4u, _metadata.size() );
    EXPECT_EQ( "Some Title", _metadata[PARAM_NAME] );
    EXPECT_EQ( "The Simpsons", _metadata[PARAM_SERIE] );
    EXPECT_EQ( "28", _metadata[PARAM_SEASON] );
    EXPECT_EQ( "20", _metadata[PARAM_EPISODE] );
}
TEST_F( FileMetaRegexTest, parse_rule04 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/srv/Downloads/Sepultura - 1996 - Roots/01 - Roots Bloody Roots.flac", _metadata ) );
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "Sepultura", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "1996", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "Roots", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "01", _metadata["track"] );
    EXPECT_EQ( "Roots Bloody Roots", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule05 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/srv/Downloads/BONAPARTE - The Return of Stravinsky Wellington (2017) [24bit]/BONAPARTE - The Return of Stravinsky Wellington (2017) [24bit]/04. Let It Ring.flac", _metadata ) );
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "BONAPARTE", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "2017", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "The Return of Stravinsky Wellington", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "04", _metadata["track"] );
    EXPECT_EQ( "Let It Ring", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule06 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::movie, cds::FileMetaRegex::parse( "video/mpeg", "/srv/Downloads/expert-title-of-interrest.mp4", _metadata ) );
    EXPECT_EQ( 1u, _metadata.size() );
    EXPECT_EQ( "expert-title-of-interrest", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule07 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::movie, cds::FileMetaRegex::parse( "video/mpeg", "/srv/Movies/Hell.or.High.Water.2016.1080p.BluRay.x264.DTS-HDC/Hell.or.High.Water.2016.1080p.BluRay.x264.DTS-HDC.mkv", _metadata ) );
    EXPECT_EQ( 2u, _metadata.size() );
    EXPECT_EQ( "Hell or High Water", _metadata[PARAM_NAME] );
    EXPECT_EQ( "2016", _metadata[PARAM_YEAR] );
}
TEST_F( FileMetaRegexTest, parse_rule08 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/foo/bar/ARTIST - ALBUM (2017) [96-24]/01 - TITLE.flac", _metadata ) );
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "ARTIST", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "2017", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "ALBUM", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "01", _metadata["track"] );
    EXPECT_EQ( "TITLE", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule09 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/foo/bar/ARTIST - ALBUM (2017) (Audio Fidelity 180g LP 24-96)/B4 - TITLE.flac", _metadata ) );
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "ARTIST", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "2017", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "ALBUM", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "B4", _metadata["track"] );
    EXPECT_EQ( "TITLE", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule10 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/foo/bar/2017 - ARTIST - ALBUM/B4 - TITLE", _metadata ) );
    EXPECT_EQ( 5u, _metadata.size() );
    EXPECT_EQ( "ARTIST", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "2017", _metadata[PARAM_YEAR] );
    EXPECT_EQ( "ALBUM", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "B4", _metadata["track"] );
    EXPECT_EQ( "TITLE", _metadata[PARAM_NAME] );
}
TEST_F( FileMetaRegexTest, parse_rule11 ) {
    std::map< std::string,std::string > _metadata;
    EXPECT_EQ( cds::NodeType::audio, cds::FileMetaRegex::parse( "audio/flac", "/foo/bar/Tom Petty & The Heartbreakers - ALBUM [96-24]/09-TITLE.flac", _metadata ) );
    EXPECT_EQ( 4u, _metadata.size() );
    EXPECT_EQ( "Tom Petty & The Heartbreakers", _metadata[PARAM_ARTIST] );
    EXPECT_EQ( "ALBUM", _metadata[PARAM_ALBUM] );
    EXPECT_EQ( "09", _metadata["track"] );
    EXPECT_EQ( "TITLE", _metadata[PARAM_NAME] );
}
}//namespace cds
