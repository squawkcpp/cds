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
#ifndef AMAZON_H
#define AMAZON_H

#include <map>
#include <list>
#include <string>

#include "gtest/gtest_prod.h"

namespace utils {

struct AmazonResult {
    std::string message;
    std::string status;
    int count = 0;
    std::list< std::map< std::string, std::string > > results;
};

/**
 * @brief The Amazon class
 */
class Amazon {
public:

    /**
     * @brief bookByIsbn
     * @param access_key
     * @param key
     * @param isbn
     */
    static AmazonResult bookByIsbn ( const std::string & access_key, const std::string & key, const std::string & isbn );

private:
    /**
     * @brief parse the response xml
     * @param response
     * @return the result
     */
    static AmazonResult parse ( const std::string & response );

    FRIEND_TEST ( AmazonTest, TestParseWithoutReviews );
    FRIEND_TEST ( AmazonTest, EmptyResult );
    FRIEND_TEST ( AmazonTest, TestCanonicalize );
    FRIEND_TEST ( AmazonTest, TestMac );
    FRIEND_TEST ( AmazonTest, TestParseResponse );

    Amazon();
    static std::string data;
    static size_t writeCallback(char* buf, size_t size, size_t nmemb, void* up);

    /**
     * Canonicalize the query string as required by Amazon.
     *
     * @param sortedParamMap    Parameter name-value pairs in lexicographical order.
     * @return                  Canonical form of query string.
     */
    static std::string canonicalize ( const std::map<std::string, std::string> & sortedParamMap );
    /**
     * @brief get_utc_time_string
     * @return
     */
    static std::string get_utc_time_string();
    /**
     * @brief generate_hmac256bit_hash
     * @param message
     * @param key_buf
     * @return
     */
    static std::string generate_hmac256bit_hash ( const char *message, const char *key_buf );
    /**
     * @brief base64
     * @param input
     * @param length
     * @return
     */
    static char * base64 ( const unsigned char *input, int length );
};
}//namespace utils
#endif // AMAZON_H
