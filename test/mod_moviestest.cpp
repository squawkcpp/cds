/*
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

#include "../src/_utils.h"
#include "../src/datastore.h"
#include "../src/modules/mod_movies.h"

#include <gtest/gtest.h>

namespace cds {
namespace mod {

TEST(ModMoviesTest, tmdb_parse) {
    std::string response = R"json(
{
"page": 1,
"total_results": 6,
"total_pages": 1,
"results": [
 {
   "vote_count": 1263,
   "id": 338766,
   "video": false,
   "vote_average": 7.2,
   "title": "Hell or High Water",
   "popularity": 58.787933,
   "poster_path": "/6YOrNBdoXvT8aC5VPLkkN6t5z0V.jpg",
   "original_language": "en",
   "original_title": "Hell or High Water",
   "genre_ids": [
     80,
     18,
     53,
     37
   ],
   "backdrop_path": "/5GbRKOQSY08U3SQXXcQAKEnL2rE.jpg",
   "adult": false,
   "overview": "A divorced dad and his ex-con brother resort to a desperate scheme in order to save their family's farm in West Texas.",
   "release_date": "2016-08-12"
 },
 {
   "vote_count": 2,
   "id": 47932,
   "video": false,
   "vote_average": 7,
   "title": "Come Hell or High Water",
   "popularity": 1.111031,
   "poster_path": "/4S2rkZGYiuilU3Hz3oP2O9sWhqb.jpg",
   "original_language": "en",
   "original_title": "Come Hell or High Water",
   "genre_ids": [
     28,
     18
   ],
   "backdrop_path": "/9e2oM9VkZNlRaRhOunrTDZDfILw.jpg",
   "adult": false,
   "overview": "Years in prison haven't erased Justin Gatewood's (Mark Redfield) quest for vengeance in this action-packed Western set in the post-Civil War era. Now that he's free, Gatewood still wants to destroy William Curry (Mike Hagan), the man responsible for his brother's death. As Gatewood involves the entire town in his bitter feud with Curry, both men's daughters (Jennifer Rouse and Kelly Potchak) find themselves caught in the middle.",
   "release_date": "2009-03-31"
 },
 {
   "vote_count": 0,
   "id": 82834,
   "video": false,
   "vote_average": 0,
   "title": "Come Hell or High Water",
   "popularity": 1.019667,
   "poster_path": "/e2s7NF611dqo9ELgJZEKltuWY6B.jpg",
   "original_language": "en",
   "original_title": "Come Hell or High Water",
   "genre_ids": [
     28
   ],
   "backdrop_path": "/bzw4v8st0DJBVi9W9gkt8ApcVTG.jpg",
   "adult": false,
   "overview": "Keith Malloy's debut film, Come Hell or High Water, shot primarily on 16mm focuses on the simplicity and beauty of bodysurfing. Keith explores the history and progression of the sport through its many unique characters. It's about taking a breath, and kicking your feet, in the big blue sea.  The film explores the history and progression of the sport of bodysurfing and the pureness that comes from riding a wave. Shot primary in 16mm, the film takes a unique look at the culture, beauty and simplicity of the sport, capturing the stories and locations of those who belong to this community.  While Malloy is most widely known for his time in the water as a surfer, his exploration into the world of bodysurfing began some 10 years ago when he wanted to reconnect with the ocean and did so through bodysurfing. Said Malloy about this project, “It’s about taking a breath and kicking your feet in the big blue sea.”",
   "release_date": "2011-09-01"
 },
 {
   "vote_count": 3,
   "id": 66012,
   "video": false,
   "vote_average": 5.5,
   "title": "Deep Purple: Come Hell or High Water",
   "popularity": 1.140141,
   "poster_path": "/7csw5STUJBmKpl4hIvgRyz24Y2d.jpg",
   "original_language": "en",
   "original_title": "Deep Purple: Come Hell or High Water",
   "genre_ids": [
     10402
   ],
   "backdrop_path": "/sOq3vmvdDnZ15vHwP0Gg5Jwa7Tf.jpg",
   "adult": false,
   "overview": "Rock legend Deep Purple recorded live at the Birmingham NEC, UK on November 9, 1993. Five members from the band's most famous line up play their hits during their 25th anniversary world tour. Note that Ritchie Blackmore dropped out soon after this particular concert.",
   "release_date": "1993-11-16"
 },
 {
   "vote_count": 0,
   "id": 347088,
   "video": false,
   "vote_average": 0,
   "title": "Come Hell or High Water: The Battle for Turkey Creek",
   "popularity": 1.016497,
   "poster_path": "/nPUsorVCYMOE6CCulW6zf7VPytz.jpg",
   "original_language": "en",
   "original_title": "Come Hell or High Water: The Battle for Turkey Creek",
   "genre_ids": [
     99
   ],
   "backdrop_path": "/9ALl32o55jp1AcO5exS6W18p3ia.jpg",
   "adult": false,
   "overview": "When the graves of former slaves are bulldozed in Mississippi, a native son returns to protect the community they settled - a place now threatened by urban sprawl, hurricanes and an unprecedented man-made disaster.",
   "release_date": "2014-04-29"
 },
 {
   "vote_count": 0,
   "id": 304112,
   "video": false,
   "vote_average": 0,
   "title": "Hell or High Water: The Story of the Nashville Rollergirls",
   "popularity": 1.011877,
   "poster_path": "/qoCJL64nOOZ33CEgLkvEGhdmsPR.jpg",
   "original_language": "en",
   "original_title": "Hell or High Water: The Story of the Nashville Rollergirls",
   "genre_ids": [
     99
   ],
   "backdrop_path": null,
   "adult": false,
   "overview": "This inspirational story follows the Nashville Rollergirls through the loss of their practice space, the move to a new game venue, injuries, relocation of key players, huge financial pressures, and even an epic flood. Ever the underdog, and with a burning in their bellies, the Nashville Rollergirls seek to prove themselves at every turn, and nothing will prevent them from achieving the goals they set. On a quest to succeed athletically on the national stage, and to keep their skater-owned-and operated organization afloat, the Nashville Rollergirls, in true Nashville spirit, refuse to accept no, can't, or won't. And in true roller derby spirit, they bring do-it-yourself to the next level.",
   "release_date": "2012-04-19"
 }
]
})json";

    auto _tmdb_result_list = ModMovies::tmdb_parse( response );
    EXPECT_EQ( 6u, _tmdb_result_list.size() );
    auto _result_1 = _tmdb_result_list.at( 0 );
    EXPECT_EQ( "338766", _result_1["id"] );
    EXPECT_EQ( "Hell or High Water", _result_1["title"] );
}

TEST(ModMoviesTest, tmdb_parse_movie) {
        std::string response = R"json(
{
 "adult": false,
 "backdrop_path": "/5GbRKOQSY08U3SQXXcQAKEnL2rE.jpg",
 "belongs_to_collection": null,
 "budget": 12000000,
 "genres": [
   {
     "id": 80,
     "name": "Crime"
   },
   {
     "id": 18,
     "name": "Drama"
   },
   {
     "id": 53,
     "name": "Thriller"
   },
   {
     "id": 37,
     "name": "Western"
   }
 ],
 "homepage": "http://hellorhighwater.movie/",
 "id": 338766,
 "imdb_id": "tt2582782",
 "original_language": "en",
 "original_title": "Hell or High Water",
 "overview": "A divorced dad and his ex-con brother resort to a desperate scheme in order to save their family's farm in West Texas.",
 "popularity": 57.787933,
 "poster_path": "/6YOrNBdoXvT8aC5VPLkkN6t5z0V.jpg",
 "production_companies": [
   {
     "name": "Sidney Kimmel Entertainment",
     "id": 737
   },
   {
     "name": "Film 44",
     "id": 20153
   },
   {
     "name": "OddLot Entertainment",
     "id": 36209
   }
 ],
 "production_countries": [
   {
     "iso_3166_1": "US",
     "name": "United States of America"
   }
 ],
 "release_date": "2016-08-12",
 "revenue": 37589296,
 "runtime": 102,
 "spoken_languages": [
   {
     "iso_639_1": "en",
     "name": "English"
   }
 ],
 "status": "Released",
 "tagline": "Blood always follows money.",
 "title": "Hell or High Water",
 "video": false,
 "vote_average": 7.2,
 "vote_count": 1276
})json";

        auto _tmdb_result = ModMovies::tmdb_parse_movie( response );
        EXPECT_EQ( "338766", _tmdb_result.id );
        EXPECT_EQ( "Hell or High Water", _tmdb_result.title );

        EXPECT_EQ( "/5GbRKOQSY08U3SQXXcQAKEnL2rE.jpg", _tmdb_result.backdrop_path );
        EXPECT_EQ( 4U, _tmdb_result.genres.size() );
        EXPECT_EQ( std::vector< std::string >( { "Crime", "Drama", "Thriller", "Western" } ), _tmdb_result.genres );
        EXPECT_EQ( "http://hellorhighwater.movie/", _tmdb_result.homepage );
        EXPECT_EQ( "338766", _tmdb_result.id );
        EXPECT_EQ( "tt2582782", _tmdb_result.imdb_id );
        EXPECT_EQ( "en", _tmdb_result.original_language );
        EXPECT_EQ( "Hell or High Water", _tmdb_result.original_title );
        EXPECT_EQ( "A divorced dad and his ex-con brother resort to a desperate scheme in order to save their family's farm in West Texas.", _tmdb_result.overview );
        EXPECT_EQ( "/6YOrNBdoXvT8aC5VPLkkN6t5z0V.jpg", _tmdb_result.poster_path );
        EXPECT_EQ( "2016-08-12", _tmdb_result.release_date );
        EXPECT_EQ( "102", _tmdb_result.runtime );
        EXPECT_EQ( "Blood always follows money.", _tmdb_result.tagline );
        EXPECT_EQ( "Hell or High Water", _tmdb_result.title );
    }
}//namespace mod
}//namespace cds

