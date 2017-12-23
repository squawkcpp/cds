///*
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.

//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//*/
//#include "amazon.h"

//#include <sstream>

//#include <openssl/engine.h>
//#include <openssl/hmac.h>

//#include <curl/curl.h> //TODO remove curl
//////#include "http/httpclient.h"
//#include "http/utils/stringutils.h"

//#include "spdlog/spdlog.h"

//#include "rapidxml_ns.hpp"

//#include "../_utils.h"
//#include "../datastore.h"

//namespace utils {

////TODO remove
//std::string Amazon::data;
//size_t Amazon::writeCallback(char* buf, size_t size, size_t nmemb, void* /*up*/) {
//    //callback must have this declaration
//    //buf is a pointer to the data that curl has for us
//    //size*nmemb is the size of the buffer

//    for (size_t c = 0; c<size*nmemb; c++) {
//        data.push_back(buf[c]);
//    }
//    return size*nmemb; //tell curl how many bytes we handled
//}

//AmazonResult Amazon::bookByIsbn( const std::string & access_key, const std::string & key, const std::string & isbn ) {

//    std::string _isbn = cds::clean_isbn ( isbn );

//    std::map< std::string, std::string > map;
//    map["AssociateTag"] = "squawk08-20";
//    map["Keywords"] = _isbn;
//    map["Operation"] = "ItemSearch";
//    map["ResponseGroup"] = "Large";
//    map["SearchIndex"] = "Books";
//    map["Service"] = "AWSECommerceService";
//    map["Timestamp"] = get_utc_time_string();
//    map["Version"] = "2009-03-31";
//    map["AWSAccessKeyId"] = access_key;

//    std::string query = canonicalize( map );

//    std::stringstream ss;
//    ss << "GET\n" <<
//          "ecs.amazonaws.com" << "\n" <<
//          "/onca/xml" << "\n" <<
//          query;

//    std::stringstream buf;
//    buf << "http://ecs.amazonaws.com/onca/xml?" << query << "&Signature=" <<
//        http::utils::UrlEscape::urlEncode( generate_hmac256bit_hash( ss.str().c_str(), key.c_str() ).c_str() );

//    data = "";
//    CURL *curl;
//      CURLcode res;

//      curl = curl_easy_init();
//      if(curl) {
//        curl_easy_setopt(curl, CURLOPT_URL, buf.str().c_str() );
//        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &Amazon::writeCallback);

//        /* Perform the request, res will get the return code */
//        res = curl_easy_perform(curl);
//        /* Check for errors */
//        if(res != CURLE_OK)
//          fprintf(stderr, "curl_easy_perform() failed: %s\n",
//                  curl_easy_strerror(res));

//        /* always cleanup */
//        curl_easy_cleanup(curl);
//      }
//      SPDLOG_TRACE(spdlog::get(cds::LOGGER), "amazon RESULT: {}", data );
//      return parse( data );
//}
//std::string Amazon::generate_hmac256bit_hash(const char *message, const char *key_buf) {
//    unsigned char* result;
//    unsigned int result_len = 32;
//    HMAC_CTX ctx;

//    result = (unsigned char*) malloc(sizeof(char) * result_len);

//    ENGINE_load_builtin_engines();
//    ENGINE_register_all_complete();

//    HMAC_CTX_init(&ctx);
//    HMAC_Init_ex(&ctx, key_buf, strlen( key_buf )/*16*/, EVP_sha256(), NULL);
//    HMAC_Update(&ctx, (unsigned char *)message, strlen( message ) );
//    HMAC_Final(&ctx, result, &result_len);
//    HMAC_CTX_cleanup(&ctx);

//    return std::string( base64( result, result_len ) );
//}

//char * Amazon::base64(const unsigned char *input, int length) {
//   BIO *bmem, *b64;
//   BUF_MEM *bptr;

//  b64 = BIO_new(BIO_f_base64());
//   bmem = BIO_new(BIO_s_mem());
//   b64 = BIO_push(b64, bmem);
//   BIO_write(b64, input, length);
//   int res = BIO_flush(b64);
//   if( res < 1 ) {
//       std::cerr << "error calculating mac." << std::endl;
//   }
//   BIO_get_mem_ptr(b64, &bptr);

//  char *buff = (char *)malloc(bptr->length);
//   memcpy(buff, bptr->data, bptr->length-1);
//   buff[bptr->length-1] = 0;

//  BIO_free_all(b64);

//  return buff;
//}
//AmazonResult Amazon::parse( const std::string & response ) {
//    AmazonResult _result_list;
//    std::string small_image, med_image, large_image, author;

//    rapidxml_ns::xml_document<> doc;
//    doc.parse<0>( const_cast< char* >( response.c_str() ) );
//    auto root_node = doc.first_node();
//    if( strcmp( root_node->name(), "ItemSearchErrorResponse" ) == 0 ) {
//        for (rapidxml_ns::xml_node<> * __r_sieblings = root_node->first_node(); __r_sieblings; __r_sieblings = __r_sieblings->next_sibling() ) {
//            if( strcmp( __r_sieblings->name(), "Error" ) == 0 ) {
//                for (rapidxml_ns::xml_node<> * __items_node = __r_sieblings->first_node(); __items_node; __items_node = __items_node->next_sibling() ) {
//                    if( strcmp( __items_node->name(), "Code" ) == 0 ) {
//                        _result_list.status = __items_node->value();
//                    } else if( strcmp( __items_node->name(), "Message" ) == 0 ) {
//                        _result_list.message = __items_node->value();
//                    }
//                }
//            }
//        }

//    } else if( strcmp( root_node->name(), "ItemSearchResponse" ) == 0 ) {
//        for (rapidxml_ns::xml_node<> * __r_sieblings = root_node->first_node(); __r_sieblings; __r_sieblings = __r_sieblings->next_sibling() ) {
//            if( strcmp( __r_sieblings->name(), "Items" ) == 0 ) {
//                for (rapidxml_ns::xml_node<> * __items_node = __r_sieblings->first_node(); __items_node; __items_node = __items_node->next_sibling() ) {
//                    if( strcmp( __items_node->name(), "Request" ) == 0 ) {
//                        for (rapidxml_ns::xml_node<> * __errors_node = __items_node->first_node(); __errors_node; __errors_node = __errors_node->next_sibling() ) {
//                            if( strcmp( __errors_node->name(), "Errors" ) == 0 ) {
//                                for (rapidxml_ns::xml_node<> * __error_node = __errors_node->first_node(); __error_node; __error_node = __error_node->next_sibling() ) {
//                                    if( strcmp( __error_node->name(), "Error" ) == 0 ) {
//                                        for (rapidxml_ns::xml_node<> * __error_values = __error_node->first_node(); __error_values; __error_values = __error_values->next_sibling() ) {
//                                            if( strcmp( __error_values->name(), "Code" ) == 0 ) {
//                                                _result_list.status = __error_values->value();
//                                            } else if( strcmp( __error_values->name(), "Message" ) == 0 ) {
//                                                _result_list.message = __error_values->value();
//                                            }
//                                        }
//                                    }
//                                }
//                            }
//                        }
//                    } else if( strcmp( __items_node->name(), "Item" ) == 0 ) {
//                        std::map< std::string, std::string > _result_item;
//                        for (rapidxml_ns::xml_node<> * __i_node = __items_node->first_node(); __i_node; __i_node = __i_node->next_sibling() ) {
//                            if( strcmp( __i_node->name(), "EditorialReviews" ) == 0 ) {
//                                for (rapidxml_ns::xml_node<> * __review_node = __i_node->first_node(); __review_node; __review_node = __review_node->next_sibling() ) {
//                                    if( strcmp( __review_node->name(), "EditorialReview" ) == 0 ) {
//                                        for (rapidxml_ns::xml_node<> * __review_content_node = __review_node->first_node(); __review_content_node; __review_content_node = __review_content_node->next_sibling() ) {
//                                            if( strcmp( __review_content_node->name(), "Content" ) == 0 ) {
//                                                _result_item[param::COMMENT] = __review_content_node->value();
//                                            }
//                                        }
//                                    }
//                                }
//                            } else if( strcmp( __i_node->name(), "SmallImage" ) == 0 ) {
//                                for (rapidxml_ns::xml_node<> * __small_image_node = __i_node->first_node(); __small_image_node; __small_image_node = __small_image_node->next_sibling() ) {
//                                    if( strcmp( __small_image_node->name(), "URL" ) == 0 ) {
//                                        small_image = __small_image_node->value();
//                                    }
//                                }
//                            } else if( strcmp( __i_node->name(), "MediumImage" ) == 0 ) {
//                                for (rapidxml_ns::xml_node<> * __med_image_node = __i_node->first_node(); __med_image_node; __med_image_node = __med_image_node->next_sibling() ) {
//                                    if( strcmp( __med_image_node->name(), "URL" ) == 0 ) {
//                                        med_image = __med_image_node->value();
//                                    }
//                                }
//                            } else if( strcmp( __i_node->name(), "LargeImage" ) == 0 ) {
//                                for (rapidxml_ns::xml_node<> * __large_image_node = __i_node->first_node(); __large_image_node; __large_image_node = __large_image_node->next_sibling() ) {
//                                    if( strcmp( __large_image_node->name(), "URL" ) == 0 ) {
//                                        large_image = __large_image_node->value();
//                                    }
//                                }
//                            } else if( strcmp( __i_node->name(), "ItemAttributes" ) == 0 ) {
//                                for (rapidxml_ns::xml_node<> * __item_attrs_node = __i_node->first_node(); __item_attrs_node; __item_attrs_node = __item_attrs_node->next_sibling() ) {
//                                    if( strcmp( __item_attrs_node->name(), "Title" ) == 0 ) {
//                                        _result_item[param::NAME] = __item_attrs_node->value();
//                                    } else if( strcmp( __item_attrs_node->name(), "Publisher" ) == 0 ) {
//                                        _result_item[param::PUBLISHER] = __item_attrs_node->value();
//                                    } else if( strcmp( __item_attrs_node->name(), "Author" ) == 0 ) {
//                                        if( !author.empty() ) author.append( ", " );
//                                        author.append( __item_attrs_node->value() );
//                                    } else if( strcmp( __item_attrs_node->name(), "PublicationDate" ) == 0 ) {
//                                        _result_item[param::DATE] = __item_attrs_node->value();
//                                    } else if( strcmp( __item_attrs_node->name(), "ISBN" ) == 0 ) {
//                                        _result_item[param::ISBN] = __item_attrs_node->value();
//                                    }
//                                }
//                            }
//                        }
//                        //set the largest cover
//                        if( !large_image.empty() ) {
//                            _result_item[param::COVER] = large_image;
//                        } else if( !med_image.empty() ) {
//                            _result_item[param::COVER] = med_image;
//                        } else if( !small_image.empty() ) {
//                            _result_item[param::COVER] = small_image;
//                        }
//                        _result_item[param::AUTHOR] = author;
//                        _result_list.results.push_back( _result_item );


//                    } else if( strcmp( __items_node->name(), "TotalResults" ) == 0 ) {
//                        _result_list.count = std::atoi( __items_node->value() );
//                    }
//                }
//            }
//        }//for loop
//    }//response root node
//    return _result_list;
//}
//std::string Amazon::canonicalize( const std::map<std::string, std::string> & sortedParamMap ) {
//    if ( sortedParamMap.empty() ) {
//        return "";
//    }

//    std::stringstream ss;
//    bool first = true;
//    for( auto itr : sortedParamMap ) {
//        if( first ) { first = false; }
//        else { ss << "&"; }
//        ss << http::utils::UrlEscape::urlEncode( itr.first.c_str() ) << "=" << http::utils::UrlEscape::urlEncode( itr.second.c_str() );
//    }

//    return ss.str();
//}
//std::string Amazon::get_utc_time_string() {
//        char timestamp_str[20];
//        time_t rawtime;
//        struct tm *ptm;

//        time(&rawtime);
//        ptm = gmtime(&rawtime);
//        sprintf(timestamp_str, "%d-%02d-%02dT%02d:%02d:%02dZ", 1900+ptm->tm_year, 1+ptm->tm_mon, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
//        return std::string(timestamp_str);
//}
//}//namespace utils
