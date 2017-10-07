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
#include "../src/utils/amazon.h"

#include <gtest/gtest.h>

namespace utils {

TEST(AmazonTest, TestCanonicalize) {

    std::map<std::string, std::string > map;
    map["Service"] = "AWSECommerceService";
    map["AWSAccessKeyId"] = "AKIAIOSFODNN7EXAMPLE";
    map["AssociateTag"] = "mytag-20";
    map["Operation"] = "ItemLookup";
    map["ItemId"] = "0679722769";
    map["ResponseGroup"] = "Images,ItemAttributes,Offers,Reviews";
    map["Version"] = "2013-08-01";
    map["Timestamp"] = "2014-08-18T12:00:00Z";

    EXPECT_EQ(Amazon::canonicalize( map ), std::string("AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE&AssociateTag=mytag-20&ItemId=0679722769&Operation=ItemLookup&ResponseGroup=Images%2CItemAttributes%2COffers%2CReviews&Service=AWSECommerceService&Timestamp=2014-08-18T12%3A00%3A00Z&Version=2013-08-01" ) );
}

TEST(AmazonTest, TestMac) {

    std::string message = "GET\n" \
            "webservices.amazon.com\n" \
            "/onca/xml\n" \
            "AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE&AssociateTag=mytag-20&ItemId=0679722769&Operation=ItemLookup&ResponseGroup=Images%2CItemAttributes%2COffers%2CReviews&Service=AWSECommerceService&Timestamp=2014-08-18T12%3A00%3A00Z&Version=2013-08-01";
    std::string key = "1234567890";

    std::string result = Amazon::generate_hmac256bit_hash( message.c_str(), key.c_str() );
    EXPECT_EQ(result, std::string("j7bZM0LXZ9eXeZruTqWm2DIvDYVUU3wxPPpp+iXxzQc=" ) );
}

TEST(AmazonTest, TestParseResponse ) {
    std::string response =
    "<?xml version=\"1.0\" ?><ItemSearchResponse xmlns=\"http://webservices.amazon.com/AWSECommerceService/2011-08-01\"><OperationRequest><RequestId>60b53e81-7579-4ec9-b984-e21527c883eb</RequestId><Arguments><Argument Name=\"AWSAccessKeyId\" Value=\"AKIAIC5IGJNCXA3YR2FA\"></Argument><Argument Name=\"AssociateTag\" Value=\"AcmeBooks\"></Argument><Argument Name=\"Keywords\" Value=\"9780786751075\"></Argument><Argument Name=\"Operation\" Value=\"ItemSearch\"></Argument><Argument Name=\"ResponseGroup\" Value=\"Large\"></Argument><Argument Name=\"SearchIndex\" Value=\"Books\"></Argument><Argument Name=\"Service\" Value=\"AWSECommerceService\"></Argument><Argument Name=\"Timestamp\" Value=\"2015-06-14T11:27:46Z\"></Argument><Argument Name=\"Version\" Value=\"2009-03-31\"></Argument><Argument Name=\"Signature\" Value=\"sXjl7B9V/vNuKa1GjnarVqzMaKYBQ0uabMqCkGWTGwU=\"></Argument></Arguments><RequestProcessingTime>0.1605550000000000</RequestProcessingTime></OperationRequest><Items><Request><IsValid>True</IsValid><ItemSearchRequest><Keywords>9780786751075</Keywords><ResponseGroup>Large</ResponseGroup><SearchIndex>Books</SearchIndex></ItemSearchRequest></Request><TotalResults>1</TotalResults><TotalPages>1</TotalPages><MoreSearchResultsUrl>http://www.amazon.com/gp/redirect.html?linkCode=xm2&amp;SubscriptionId=AKIAIC5IGJNCXA3YR2FA&amp;location=http%3A%2F%2Fwww.amazon.com%2Fgp%2Fsearch%3Fkeywords%3D9780786751075%26url%3Dsearch-alias%253Dstripbooks&amp;tag=AcmeBooks&amp;creative=386001&amp;camp=2025</MoreSearchResultsUrl><Item><ASIN>B004WOPHN0</ASIN><DetailPageURL>http://www.amazon.com/Low-Slow-Master-Barbecue-Lessons-ebook/dp/B004WOPHN0%3FSubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D165953%26creativeASIN%3DB004WOPHN0</DetailPageURL><ItemLinks><ItemLink><Description>Technical Details</Description><URL>http://www.amazon.com/Low-Slow-Master-Barbecue-Lessons-ebook/dp/tech-data/B004WOPHN0%3FSubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink><ItemLink><Description>Add To Baby Registry</Description><URL>http://www.amazon.com/gp/registry/baby/add-item.html%3Fasin.0%3DB004WOPHN0%26SubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink><ItemLink><Description>Add To Wedding Registry</Description><URL>http://www.amazon.com/gp/registry/wedding/add-item.html%3Fasin.0%3DB004WOPHN0%26SubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink><ItemLink><Description>Add To Wishlist</Description><URL>http://www.amazon.com/gp/registry/wishlist/add-item.html%3Fasin.0%3DB004WOPHN0%26SubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink><ItemLink><Description>Tell A Friend</Description><URL>http://www.amazon.com/gp/pdp/taf/B004WOPHN0%3FSubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink><ItemLink><Description>All Customer Reviews</Description><URL>http://www.amazon.com/review/product/B004WOPHN0%3FSubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink><ItemLink><Description>All Offers</Description><URL>http://www.amazon.com/gp/offer-listing/B004WOPHN0%3FSubscriptionId%3DAKIAIC5IGJNCXA3YR2FA%26tag%3DAcmeBooks%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3DB004WOPHN0</URL></ItemLink></ItemLinks><SalesRank>60602</SalesRank><SmallImage><URL>http://ecx.images-amazon.com/images/I/" \
    "51IUbMMktGL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">59</Width></SmallImage><MediumImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL._SL160_.jpg</URL><Height Units=\"pixels\">160</Height><Width Units=\"pixels\">126</Width></MediumImage><LargeImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL.jpg</URL><Height Units=\"pixels\">500</Height><Width Units=\"pixels\">394</Width></LargeImage><ImageSets><ImageSet Category=\"primary\"><SwatchImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL._SL30_.jpg</URL><Height Units=\"pixels\">30</Height><Width Units=\"pixels\">24</Width></SwatchImage><SmallImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">59</Width></SmallImage><ThumbnailImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">59</Width></ThumbnailImage><TinyImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL._SL110_.jpg</URL><Height Units=\"pixels\">110</Height><Width Units=\"pixels\">87</Width></TinyImage><MediumImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL._SL160_.jpg</URL><Height Units=\"pixels\">160</Height><Width Units=\"pixels\">126</Width></MediumImage><LargeImage><URL>http://ecx.images-amazon.com/images/I/51IUbMMktGL.jpg</URL><Height Units=\"pixels\">500</Height><Width Units=\"pixels\">394</Width></LargeImage></ImageSet></ImageSets><ItemAttributes><Author>Gary Wiviott</Author><Author>Colleen Rush</Author><Binding>Kindle Edition</Binding><EISBN>9780786751075</EISBN><Format>Kindle eBook</Format><Label>Running Press</Label><Languages><Language><Name>English</Name><Type>Published</Type></Language></Languages><Manufacturer>Running Press</Manufacturer><NumberOfPages>258</NumberOfPages><ProductGroup>eBooks</ProductGroup><ProductTypeName>ABIS_EBOOKS</ProductTypeName><PublicationDate>2009-08-05</PublicationDate><Publisher>Running Press</Publisher><ReleaseDate>2009-08-05</ReleaseDate><Studio>Running Press</Studio><Title>Low &amp; Slow: Master the Art of Barbecue in 5 Easy Lessons</Title></ItemAttributes><CustomerReviews><IFrameURL>http://www.amazon.com/reviews/iframe?akid=AKIAIC5IGJNCXA3YR2FA&amp;alinkCode=xm2&amp;asin=B004WOPHN0&amp;atag=AcmeBooks&amp;exp=2015-06-15T11%3A27%3A47Z&amp;v=2&amp;sig=oNHhXbh55uDr5buYFYAhb%2BQGN%2F466E1f973rtwFPjL0%3D</IFrameURL><HasReviews>true</HasReviews></CustomerReviews><EditorialReviews><EditorialReview><Source>Product Description</Source><Content>&lt;div&gt;&lt;p&gt;Step away from the propane tank. Surrender all of your notions about barbecue. Forget everything you've ever learned about cooking with charcoal and fire. It is all wrong. Get it right with the \"Five Easy Lessons\" program, which includes over 130 recipes and step-by-step instructions for setting up and cooking low and slow on a Weber Smokey Mountain, an offset smoker, or a kettle grill.&lt;P&gt;This program is guided by a singular philosophy: Keep It Simple, Stupid. Do exactly as Gary says, don't even think about opening the lid before it's time, and you will learn:&lt;P&gt;o What gear you do and, more importantly, don't need&lt;BR&gt;o Exactly how to start and maintain a proper fire (without lighter fluid)&lt;BR&gt;o All about marinades, brines, and rubs&lt;BR&gt;o To use your senses and trust your instincts (instead of thermometers)&lt;BR&gt;o How to make delicious, delicious barbecue&lt;/p&gt;&lt;/div&gt;</Content><IsLinkSuppressed>0</IsLinkSuppressed></EditorialReview><EditorialReview><Source>Product Description</Source><Content>&lt;div&gt;&lt;p&gt;Step away from the propane tank. Surrender all of your notions about barbecue. Forget everything you've ever learned about cooking with charcoal and fire. It is all wrong. Get it right with the \"Five Easy Lessons\" program, which includes over 130 recipes and step-by-step instructions for setting up and cooking low and slow on a Weber Smokey Mountain, an offset smoker, or a kettle grill.&lt;P&gt;This program is guided by a singular philosophy: Keep It Sim" \
    "ple, Stupid. Do exactly as Gary says, don't even think about opening the lid before it's time, and you will learn:&lt;P&gt;o What gear you do and, more importantly, don't need&lt;BR&gt;o Exactly how to start and maintain a proper fire (without lighter fluid)&lt;BR&gt;o All about marinades, brines, and rubs&lt;BR&gt;o To use your senses and trust your instincts (instead of thermometers)&lt;BR&gt;o How to make delicious, delicious barbecue&lt;/p&gt;&lt;/div&gt;</Content><IsLinkSuppressed>0</IsLinkSuppressed></EditorialReview></EditorialReviews><SimilarProducts><SimilarProduct><ASIN>B00VPPSRWM</ASIN><Title>Low &amp; Slow 2: The Art of Barbecue, Smoke-Roasting, and Basic Curing</Title></SimilarProduct><SimilarProduct><ASIN>B004MME6MK</ASIN><Title>Smokin' with Myron Mixon: Backyard 'Cue Made Simple from the Winningest Man in Barbecue</Title></SimilarProduct><SimilarProduct><ASIN>B007RZ07B8</ASIN><Title>Slow Fire: The Beginner's Guide to Lip-Smacking Barbecue</Title></SimilarProduct><SimilarProduct><ASIN>B00K5Y22QG</ASIN><Title>Weber's Smoke: A Guide to Smoke Cooking for Everyone and Any Grill</Title></SimilarProduct><SimilarProduct><ASIN>B00N6PFBDW</ASIN><Title>Franklin Barbecue: A Meat-Smoking Manifesto</Title></SimilarProduct></SimilarProducts><BrowseNodes><BrowseNode><BrowseNodeId>4246</BrowseNodeId><Name>Barbecuing &amp; Grilling</Name><Ancestors><BrowseNode><BrowseNodeId>4245</BrowseNodeId><Name>Outdoor Cooking</Name><Ancestors><BrowseNode><BrowseNodeId>6</BrowseNodeId><Name>Cookbooks, Food &amp; Wine</Name><Ancestors><BrowseNode><BrowseNodeId>1000</BrowseNodeId><Name>Subjects</Name><IsCategoryRoot>1</IsCategoryRoot><Ancestors><BrowseNode><BrowseNodeId>283155</BrowseNodeId><Name>Books</Name></BrowseNode></Ancestors></BrowseNode></Ancestors></BrowseNode></Ancestors></BrowseNode></Ancestors></BrowseNode><BrowseNode><BrowseNodeId>156196011</BrowseNodeId><Name>Outdoor Cooking</Name><Ancestors><BrowseNode><BrowseNodeId>156154011</BrowseNodeId><Name>Cookbooks, Food &amp; Wine</Name><Ancestors><BrowseNode><BrowseNodeId>154606011</BrowseNodeId><Name>Kindle eBooks</Name><Ancestors><BrowseNode><BrowseNodeId>133141011</BrowseNodeId><Name>Categories</Name><IsCategoryRoot>1</IsCategoryRoot><Ancestors><BrowseNode><BrowseNodeId>133140011</BrowseNodeId><Name>Kindle Store</Name></BrowseNode></Ancestors></BrowseNode></Ancestors></BrowseNode></Ancestors></BrowseNode></Ancestors></BrowseNode></BrowseNodes></Item></Items></ItemSearchResponse>";

    auto book = Amazon::parse( response );

    EXPECT_EQ( 1U, book.count );
    EXPECT_EQ( 1U, book.results.size() );
    auto firstBook = book.results.begin();

    EXPECT_EQ((*firstBook)[param::NAME], std::string("Low & Slow: Master the Art of Barbecue in 5 Easy Lessons" ) );
    EXPECT_EQ((*firstBook)[param::AUTHOR], std::string("Gary Wiviott, Colleen Rush" ) );
    EXPECT_EQ((*firstBook)[param::COVER], std::string("http://ecx.images-amazon.com/images/I/51IUbMMktGL.jpg" ) );
    EXPECT_EQ((*firstBook)[param::COMMENT], std::string("<div><p>Step away from the propane tank. Surrender all of your notions about barbecue. Forget everything you've ever learned about cooking with charcoal and fire. It is all wrong. Get it right with the \"Five Easy Lessons\" program, which includes over 130 recipes and step-by-step instructions for setting up and cooking low and slow on a Weber Smokey Mountain, an offset smoker, or a kettle grill.<P>This program is guided by a singular philosophy: Keep It Simple, Stupid. Do exactly as Gary says, don't even think about opening the lid before it's time, and you will learn:<P>o What gear you do and, more importantly, don't need<BR>o Exactly how to start and maintain a proper fire (without lighter fluid)<BR>o All about marinades, brines, and rubs<BR>o To use your senses and trust your instincts (instead of thermometers)<BR>o How to make delicious, delicious barbecue</p></div>" ) );

    EXPECT_EQ((*firstBook)[param::DATE], "2009-08-05" );
    EXPECT_EQ((*firstBook)[param::PUBLISHER], std::string("Running Press" ) );
}
TEST(AmazonTest, EmptyResult ) {
    std::string response =
            "<?xml version=\"1.0\" ?><ItemSearchResponse xmlns=\"http://webservices.amazon.com/AWSECommerceService/2011-08-01\"><OperationRequest><RequestId>e058d2bc-aab9-4ef8-99a1-44b3e2140aae</RequestId><Arguments><Argument Name=\"AWSAccessKeyId\" Value=\"AKIAJL7OW25HI5DRKZJQ\"></Argument><Argument Name=\"AssociateTag\" Value=\"squawk08-20\"></Argument><Argument Name=\"Keywords\" Value=\"9781402798474\"></Argument><Argument Name=\"Operation\" Value=\"ItemSearch\"></Argument><Argument Name=\"ResponseGroup\" Value=\"Large\"></Argument><Argument Name=\"SearchIndex\" Value=\"Books\"></Argument><Argument Name=\"Service\" Value=\"AWSECommerceService\"></Argument><Argument Name=\"Timestamp\" Value=\"2017-02-06T21:41:44Z\"></Argument><Argument Name=\"Version\" Value=\"2009-03-31\"></Argument><Argument Name=\"Signature\" Value=\"ULjNNP/spxXDfItD4c7j8XOEbeGQM4zZmqv+NLONalA=\"></Argument></Arguments><RequestProcessingTime>0.0154423260000000</RequestProcessingTime></OperationRequest><Items><Request><IsValid>True</IsValid><ItemSearchRequest><Keywords>9781402798474</Keywords><ResponseGroup>Large</ResponseGroup><SearchIndex>Books</SearchIndex></ItemSearchRequest><Errors><Error><Code>AWS.ECommerceService.NoExactMatches</Code><Message>We did not find any matches for your request.</Message></Error></Errors></Request><TotalResults>0</TotalResults><TotalPages>0</TotalPages><MoreSearchResultsUrl>https://www.amazon.com/gp/redirect.html?linkCode=xm2&amp;SubscriptionId=AKIAJL7OW25HI5DRKZJQ&amp;location=https%3A%2F%2Fwww.amazon.com%2Fgp%2Fsearch%3Fkeywords%3D9781402798474%26url%3Dsearch-alias%253Dstripbooks&amp;tag=squawk08-20&amp;creative=386001&amp;camp=2025</MoreSearchResultsUrl></Items></ItemSearchResponse>";

    auto book = Amazon::parse( response );
    EXPECT_EQ(book.status, "AWS.ECommerceService.NoExactMatches" );
    EXPECT_EQ(book.count, 0 );
}

TEST(AmazonTest, TestParseWithoutReviews ) {
    std::string response =
            "<?xml version=\"1.0\" ?><ItemSearchResponse xmlns=\"http://webservices.amazon.com/AWSECommerceService/2011-08-01\"><OperationRequest><RequestId>2bdc3114-f1cf-436a-9ce1-6d6192a07db7</RequestId><Arguments><Argument Name=\"AWSAccessKeyId\" Value=\"AKIAJL7OW25HI5DRKZJQ\"></Argument><Argument Name=\"AssociateTag\" Value=\"squawk08-20\"></Argument><Argument Name=\"Keywords\" Value=\"9783831022632\"></Argument><Argument Name=\"Operation\" Value=\"ItemSearch\"></Argument><Argument Name=\"ResponseGroup\" Value=\"Large\"></Argument><Argument Name=\"SearchIndex\" Value=\"Books\"></Argument><Argument Name=\"Service\" Value=\"AWSECommerceService\"></Argument><Argument Name=\"Timestamp\" Value=\"2017-02-04T10:58:21Z\"></Argument><Argument Name=\"Version\" Value=\"2009-03-31\"></Argument><Argument Name=\"Signature\" Value=\"dh/+VTsgWIS0t/HZkaS7bO/JMIEAQpR+sNCRUY/TLrY=\"></Argument></Arguments><RequestProcessingTime>0.1307347480000000</RequestProcessingTime></OperationRequest><Items><Request><IsValid>True</IsValid><ItemSearchRequest><Keywords>9783831022632</Keywords><ResponseGroup>Large</ResponseGroup><SearchIndex>Books</SearchIndex></ItemSearchRequest></Request><TotalResults>1</TotalResults><TotalPages>1</TotalPages><MoreSearchResultsUrl>https://www.amazon.com/gp/redirect.html?linkCode=xm2&amp;SubscriptionId=AKIAJL7OW25HI5DRKZJQ&amp;location=https%3A%2F%2Fwww.amazon.com%2Fgp%2Fsearch%3Fkeywords%3D9783831022632%26url%3Dsearch-alias%253Dstripbooks&amp;tag=squawk08-20&amp;creative=386001&amp;camp=2025</MoreSearchResultsUrl><Item><ASIN>3831022631</ASIN><DetailPageURL>https://www.amazon.com/Jamies-15-Minuten-K%C3%BCche-Yulo-inc/dp/3831022631%3FSubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D165953%26creativeASIN%3D3831022631</DetailPageURL><ItemLinks><ItemLink><Description>Technical Details</Description><URL>https://www.amazon.com/Jamies-15-Minuten-K%C3%BCche-Yulo-inc/dp/tech-data/3831022631%3FSubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink><ItemLink><Description>Add To Baby Registry</Description><URL>https://www.amazon.com/gp/registry/baby/add-item.html%3Fasin.0%3D3831022631%26SubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink><ItemLink><Description>Add To Wedding Registry</Description><URL>https://www.amazon.com/gp/registry/wedding/add-item.html%3Fasin.0%3D3831022631%26SubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink><ItemLink><Description>Add To Wishlist</Description><URL>https://www.amazon.com/gp/registry/wishlist/add-item.html%3Fasin.0%3D3831022631%26SubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink><ItemLink><Description>Tell A Friend</Description><URL>https://www.amazon.com/gp/pdp/taf/3831022631%3FSubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink><ItemLink><Description>All Customer Reviews</Description><URL>https://www.amazon.com/review/product/3831022631%3FSubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink><ItemLink><Description>All Offers</Description><URL>https://www.amazon.com/gp/offer-listing/3831022631%3FSubscriptionId%3DAKIAJL7OW25HI5DRKZJQ%26tag%3Dsquawk08-20%26linkCode%3Dxm2%26camp%3D2025%26creative%3D386001%26creativeASIN%3D3831022631</URL></ItemLink></ItemLinks><SalesRank>7073803</SalesRank><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></SmallImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL160_.jpg</URL><Height Units=\"pixels\">160</Height><Width Units=\"pixels\">124</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL.jpg</URL><Height Units=\"pixels\">500</Height><Width Units=\"pixels\">387</Width></LargeImage><ImageSets><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/41PiUz8i1YL._SL30_.jpg</URL><Height Units=\"pixels\">30</Height><Width Units=\"pixels\">23</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/41PiUz8i1YL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/41PiUz8i1YL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/41PiUz8i1YL._SL110_.jpg</URL><Height Units=\"pixels\">110</Height><Width Units=\"pixels\">84</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/41PiUz8i1YL._SL160_.jpg</URL><Height Units=\"pixels\">160</Height><Width Units=\"pixels\">123</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/41PiUz8i1YL.jpg</URL><Height Units=\"pixels\">500</Height><Width Units=\"pixels\">384</Width></LargeImage></ImageSet><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/51SnYw7MwEL._SL30_.jpg</URL><Height Units=\"pixels\">20</Height><Width Units=\"pixels\">30</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/51SnYw7MwEL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/51SnYw7MwEL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/51SnYw7MwEL._SL110_.jpg</URL><Height Units=\"pixels\">72</Height><Width Units=\"pixels\">110</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/51SnYw7MwEL._SL160_.jpg</URL><Height Units=\"pixels\">104</Height><Width Units=\"pixels\">160</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/51SnYw7MwEL.jpg</URL><Height Units=\"pixels\">325</Height><Width Units=\"pixels\">500</Width></LargeImage></ImageSet><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/51QLuohtMwL._SL30_.jpg</URL><Height Units=\"pixels\">20</Height><Width Units=\"pixels\">30</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/51QLuohtMwL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/51QLuohtMwL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/51QLuohtMwL._SL110_.jpg</URL><Height Units=\"pixels\">72</Height><Width Units=\"pixels\">110</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/51QLuohtMwL._SL160_.jpg</URL><Height Units=\"pixels\">104</Height><Width Units=\"pixels\">160</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/51QLuohtMwL.jpg</URL><Height Units=\"pixels\">325</Height><Width Units=\"pixels\">500</Width></LargeImage></ImageSet><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/51vuR%2Bt9H2L._SL30_.jpg</URL><Height Units=\"pixels\">20</Height><Width Units=\"pixels\">30</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/51vuR%2Bt9H2L._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/51vuR%2Bt9H2L._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/51vuR%2Bt9H2L._SL110_.jpg</URL><Height Units=\"pixels\">72</Height><Width Units=\"pixels\">110</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/51vuR%2Bt9H2L._SL160_.jpg</URL><Height Units=\"pixels\">104</Height><Width Units=\"pixels\">160</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/51vuR%2Bt9H2L.jpg</URL><Height Units=\"pixels\">325</Height><Width Units=\"pixels\">500</Width></LargeImage></ImageSet><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/61SK-G895iL._SL30_.jpg</URL><Height Units=\"pixels\">30</Height><Width Units=\"pixels\">23</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/61SK-G895iL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/61SK-G895iL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/61SK-G895iL._SL110_.jpg</URL><Height Units=\"pixels\">110</Height><Width Units=\"pixels\">85</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/61SK-G895iL._SL160_.jpg</URL><Height Units=\"pixels\">160</Height><Width Units=\"pixels\">124</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/61SK-G895iL.jpg</URL><Height Units=\"pixels\">500</Height><Width Units=\"pixels\">387</Width></LargeImage></ImageSet><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/51ZGBcVlDWL._SL30_.jpg</URL><Height Units=\"pixels\">20</Height><Width Units=\"pixels\">30</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/51ZGBcVlDWL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/51ZGBcVlDWL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/51ZGBcVlDWL._SL110_.jpg</URL><Height Units=\"pixels\">72</Height><Width Units=\"pixels\">110</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/51ZGBcVlDWL._SL160_.jpg</URL><Height Units=\"pixels\">104</Height><Width Units=\"pixels\">160</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/51ZGBcVlDWL.jpg</URL><Height Units=\"pixels\">326</Height><Width Units=\"pixels\">500</Width></LargeImage></ImageSet><ImageSet Category=\"variant\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/510K0u1M1tL._SL30_.jpg</URL><Height Units=\"pixels\">20</Height><Width Units=\"pixels\">30</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/510K0u1M1tL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/510K0u1M1tL._SL75_.jpg</URL><Height Units=\"pixels\">49</Height><Width Units=\"pixels\">75</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/510K0u1M1tL._SL110_.jpg</URL><Height Units=\"pixels\">72</Height><Width Units=\"pixels\">110</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/510K0u1M1tL._SL160_.jpg</URL><Height Units=\"pixels\">104</Height><Width Units=\"pixels\">160</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/510K0u1M1tL.jpg</URL><Height Units=\"pixels\">326</Height><Width Units=\"pixels\">500</Width></LargeImage></ImageSet><ImageSet Category=\"primary\"><SwatchImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL30_.jpg</URL><Height Units=\"pixels\">30</Height><Width Units=\"pixels\">23</Width></SwatchImage><SmallImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></SmallImage><ThumbnailImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL75_.jpg</URL><Height Units=\"pixels\">75</Height><Width Units=\"pixels\">58</Width></ThumbnailImage><TinyImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL110_.jpg</URL><Height Units=\"pixels\">110</Height><Width Units=\"pixels\">85</Width></TinyImage><MediumImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL._SL160_.jpg</URL><Height Units=\"pixels\">160</Height><Width Units=\"pixels\">124</Width></MediumImage><LargeImage><URL>https://images-na.ssl-images-amazon.com/images/I/51FNRCVAASL.jpg</URL><Height Units=\"pixels\">500</Height><Width Units=\"pixels\">387</Width></LargeImage></ImageSet></ImageSets><ItemAttributes><Author>Imported by Yulo inc.</Author><Binding>Hardcover</Binding><EAN>9783831022632</EAN><EANList><EANListElement>9783831022632</EANListElement></EANList><ISBN>3831022631</ISBN><ItemDimensions><Height Units=\"hundredths-inches\">1004</Height><Length Units=\"hundredths-inches\">787</Length><Width Units=\"hundredths-inches\">118</Width></ItemDimensions><Label>Jamies 15-Minuten-Küche: Blitzschnell, gesund und superlecker</Label><Languages><Language><Name>German</Name><Type>Published</Type></Language><Language><Name>German</Name><Type>Original Language</Type></Language><Language><Name>German</Name><Type>Unknown</Type></Language></Languages><Manufacturer>Jamies 15-Minuten-Küche: Blitzschnell, gesund und superlecker</Manufacturer><MPN>978-3-8310-2263-2</MPN><PackageDimensions><Height Units=\"hundredths-inches\">102</Height><Length Units=\"hundredths-inches\">1008</Length><Weight Units=\"hundredths-pounds\">265</Weight><Width Units=\"hundredths-inches\">795</Width></PackageDimensions><PartNumber>978-3-8310-2263-2</PartNumber><ProductGroup>Book</ProductGroup><ProductTypeName>ABIS_BOOK</ProductTypeName><PublicationDate>2014</PublicationDate><Publisher>Jamies 15-Minuten-Küche: Blitzschnell, gesund und superlecker</Publisher><Studio>Jamies 15-Minuten-Küche: Blitzschnell, gesund und superlecker</Studio><Title>Jamies 15-Minuten-Küche</Title></ItemAttributes><OfferSummary><LowestNewPrice><Amount>2106</Amount><CurrencyCode>USD</CurrencyCode><FormattedPrice>$21.06</FormattedPrice></LowestNewPrice><LowestUsedPrice><Amount>1903</Amount><CurrencyCode>USD</CurrencyCode><FormattedPrice>$19.03</FormattedPrice></LowestUsedPrice><TotalNew>9</TotalNew><TotalUsed>4</TotalUsed><TotalCollectible>0</TotalCollectible><TotalRefurbished>0</TotalRefurbished></OfferSummary><Offers><TotalOffers>0</TotalOffers><TotalOfferPages>0</TotalOfferPages><MoreOffersUrl>0</MoreOffersUrl></Offers><CustomerReviews><IFrameURL>https://www.amazon.com/reviews/iframe?akid=AKIAJL7OW25HI5DRKZJQ&amp;alinkCode=xm2&amp;asin=3831022631&amp;atag=squawk08-20&amp;exp=2017-02-05T10%3A58%3A21Z&amp;v=2&amp;sig=ml3hvkxU1XvNZsVFQVSkhffTh7gdFe05Qd28BAHiMfc%3D</IFrameURL><HasReviews>false</HasReviews></CustomerReviews><BrowseNodes><BrowseNode><BrowseNodeId>1000</BrowseNodeId><Name>Subjects</Name><IsCategoryRoot>1</IsCategoryRoot><Children><BrowseNode><BrowseNodeId>1</BrowseNodeId><Name>Arts &amp; Photography</Name></BrowseNode><BrowseNode><BrowseNodeId>2</BrowseNodeId><Name>Biographies &amp; Memoirs</Name></BrowseNode><BrowseNode><BrowseNodeId>3</BrowseNodeId><Name>Business &amp; Money</Name></BrowseNode><BrowseNode><BrowseNodeId>3248857011</BrowseNodeId><Name>Calendars</Name></BrowseNode><BrowseNode><BrowseNodeId>4</BrowseNodeId><Name>Children's Books</Name></BrowseNode><BrowseNode><BrowseNodeId>12290</BrowseNodeId><Name>Christian Books &amp; Bibles</Name></BrowseNode><BrowseNode><BrowseNodeId>4366</BrowseNodeId><Name>Comics &amp; Graphic Novels</Name></BrowseNode><BrowseNode><BrowseNodeId>5</BrowseNodeId><Name>Computers &amp; Technology</Name></BrowseNode><BrowseNode><BrowseNodeId>6</BrowseNodeId><Name>Cookbooks, Food &amp; Wine</Name></BrowseNode><BrowseNode><BrowseNodeId>48</BrowseNodeId><Name>Crafts, Hobbies &amp; Home</Name></BrowseNode><BrowseNode><BrowseNodeId>8975347011</BrowseNodeId><Name>Education &amp; Teaching</Name></BrowseNode><BrowseNode><BrowseNodeId>173507</BrowseNodeId><Name>Engineering &amp; Transportation</Name></BrowseNode><BrowseNode><BrowseNodeId>301889</BrowseNodeId><Name>Gay &amp; Lesbian</Name></BrowseNode><BrowseNode><BrowseNodeId>10</BrowseNodeId><Name>Health, Fitness &amp; Dieting</Name></BrowseNode><BrowseNode><BrowseNodeId>9</BrowseNodeId><Name>History</Name></BrowseNode><BrowseNode><BrowseNodeId>86</BrowseNodeId><Name>Humor &amp; Entertainment</Name></BrowseNode><BrowseNode><BrowseNodeId>10777</BrowseNodeId><Name>Law</Name></BrowseNode><BrowseNode><BrowseNodeId>17</BrowseNodeId><Name>Literature &amp; Fiction</Name></BrowseNode><BrowseNode><BrowseNodeId>173514</BrowseNodeId><Name>Medical Books</Name></BrowseNode><BrowseNode><BrowseNodeId>18</BrowseNodeId><Name>Mystery, Thriller &amp; Suspense</Name></BrowseNode><BrowseNode><BrowseNodeId>20</BrowseNodeId><Name>Parenting &amp; Relationships</Name></BrowseNode><BrowseNode><BrowseNodeId>3377866011</BrowseNodeId><Name>Politics &amp; Social Sciences</Name></BrowseNode><BrowseNode><BrowseNodeId>21</BrowseNodeId><Name>Reference</Name></BrowseNode><BrowseNode><BrowseNodeId>22</BrowseNodeId><Name>Religion &amp; Spirituality</Name></BrowseNode><BrowseNode><BrowseNodeId>23</BrowseNodeId><Name>Romance</Name></BrowseNode><BrowseNode><BrowseNodeId>75</BrowseNodeId><Name>Science &amp; Math</Name></BrowseNode><BrowseNode><BrowseNodeId>25</BrowseNodeId><Name>Science Fiction &amp; Fantasy</Name></BrowseNode><BrowseNode><BrowseNodeId>4736</BrowseNodeId><Name>Self-Help</Name></BrowseNode><BrowseNode><BrowseNodeId>26</BrowseNodeId><Name>Sports &amp; Outdoors</Name></BrowseNode><BrowseNode><BrowseNodeId>28</BrowseNodeId><Name>Teen &amp; Young Adult</Name></BrowseNode><BrowseNode><BrowseNodeId>5267710011</BrowseNodeId><Name>Test Preparation</Name></BrowseNode><BrowseNode><BrowseNodeId>27</BrowseNodeId><Name>Travel</Name></BrowseNode></Children><Ancestors><BrowseNode><BrowseNodeId>283155</BrowseNodeId><Name>Books</Name></BrowseNode></Ancestors></BrowseNode></BrowseNodes></Item></Items></ItemSearchResponse>";

    auto book = Amazon::parse( response );
    auto firstBook = book.results.begin();
    EXPECT_EQ((*firstBook)[param::NAME], std::string("Jamies 15-Minuten-Küche" ) );
}

TEST(AmazonTest, TestParseErrorResponse ) {
    std::string response = R"xml(<?xml version="1.0"?>
<ItemSearchErrorResponse xmlns="http://ecs.amazonaws.com/doc/2009-03-31/"><Error><Code>RequestThrottled</Code><Message>AWS Access Key ID: AKIAJL7OW25HI5DRKZJQ. You are submitting requests too quickly. Please retry your requests at a slower rate.</Message></Error><RequestID>a7075099-03f6-4520-a958-13a6c5fc04ba</RequestID></ItemSearchErrorResponse>)xml";

    auto __response = Amazon::parse( response );
    EXPECT_EQ( std::string("RequestThrottled" ), __response.status );
    EXPECT_EQ( std::string( "AWS Access Key ID: AKIAJL7OW25HI5DRKZJQ. You are submitting requests too quickly. Please retry your requests at a slower rate." ), __response.message );
}
}//namespace amazon
