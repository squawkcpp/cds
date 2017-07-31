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

#include "../_utils.h"
#include "../modules/mod_albums.h"

#include <gtest/gtest.h>

namespace cds {
namespace mod {

TEST(MusicbrainzTest, parse_artist_mbid) {

    const char* body = R"xml(
            <?xml version="1.0" encoding="UTF-8" standalone="yes"?><metadata created="2017-07-21T20:23:16.855Z" xmlns="http://musicbrainz.org/ns/mmd-2.0#" xmlns:ext="http://musicbrainz.org/ns/ext#-2.0"><artist-list count="769" offset="0"><artist id="79491354-3d83-40e3-9d8e-7592d58d790a" type="Group" ext:score="100"><name>Deep Purple</name><sort-name>Deep Purple</sort-name><country>GB</country><area id="8a754a16-0027-3a29-b6d7-2b40ea0481ed"><name>United Kingdom</name><sort-name>United Kingdom</sort-name></area><begin-area id="93e36643-acf7-4fb4-8760-de9d6bebfe4c"><name>Hertford</name><sort-name>Hertford</sort-name></begin-area><disambiguation>English rock band</disambiguation><life-span><begin>1968</begin><ended>false</ended></life-span><alias-list><alias sort-name="ジープパープル">ジープパープル</alias><alias sort-name="ジープ・パープル">ジープ・パープル</alias><alias sort-name="ディープパープル">ディープパープル</alias><alias sort-name="ディープ・パープル">ディープ・パープル</alias><alias sort-name="Deep Purpel">Deep Purpel</alias><alias sort-name="Deep Purple and The Royal Philharmonic Orchestra">Deep Purple and The Royal Philharmonic Orchestra</alias><alias sort-name="Dee Purple">Dee Purple</alias><alias sort-name="Deep Purple">Depp Purple</alias><alias sort-name="Dep Purpel">Dep Purpel</alias><alias sort-name="Dep Purple">Dep Purple</alias><alias sort-name="DP">DP</alias><alias sort-name="Paice, Blackmore ,Gillan, Glover, Lord">Paice, Blackmore ,Gillan, Glover, Lord</alias><alias sort-name="Purple" type="Search hint">Purple</alias><alias sort-name="深い紫">深い紫</alias></alias-list><tag-list><tag count="2"><name>rock</name></tag><tag count="6"><name>progressive rock</name></tag><tag count="0"><name>70s</name></tag><tag count="0"><name>80s</name></tag><tag count="4"><name>heavy metal</name></tag><tag count="1"><name>metal</name></tag><tag count="2"><name>british</name></tag><tag count="0"><name>uk</name></tag><tag count="0"><name>60s</name></tag><tag count="7"><name>hard rock</name></tag><tag count="2"><name>blues rock</name></tag><tag count="0"><name>deep purple family</name></tag><tag count="0"><name>collection</name></tag><tag count="0"><name>classic rock</name></tag><tag count="1"><name>symphonic rock</name></tag><tag count="0"><name>english</name></tag><tag count="1"><name>aor</name></tag><tag count="1"><name>symphonic prog</name></tag><tag count="0"><name>classic pop and rock</name></tag><tag count="0"><name>deep purple</name></tag><tag count="0"><name>deep purple live in berlin 1973 dvd video</name></tag><tag count="0"><name>want to listen</name></tag><tag count="0"><name>anthology</name></tag></tag-list></artist><artist id="191b19f0-019f-48d5-99f3-8a3195f565fa" type="Person" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><gender>male</gender><area id="f03d09b3-39dc-4083-afd6-159e3f0d462f"><name>London</name><sort-name>London</sort-name></area><disambiguation>UK Garage producer from London</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="8387953c-bd3b-4965-9465-e31aacf42d01" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><disambiguation>South African</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="722e7781-9462-42cc-b59b-92b61251c1db" type="Group" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><area id="f934c8da-e40e-4056-8f8c-212e68fdcaec"><name>Texas</name><sort-name>Texas</sort-name></area><disambiguation>Purple from SouthEast Texas: Hanna Brewer, Taylor Busby, Tyler "Smitty" Smith</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="5e358b0e-75fd-410c-b440-4c416e35198f" type="Group" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><area id="f934c8da-e40e-4056-8f8c-212e68fdcaec"><name>Texas</name><sort-name>Texas</sort-name></area><disambiguation>3 piece garage band from texas</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="f8218e7e-4dc4-48d9-af3a-71a0a3eb3e19" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><disambiguation>US Rapper</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="83d275b0-2c8a-40f8-b585-398b3f08306f" type="Person" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><gender>male</gender><begin-area id="309f8f03-51b5-4640-b4cf-d1a5e022ccf4"><name>Porto</name><sort-name>Porto</sort-name></begin-area><disambiguation>The Porto-born, Berlin-based</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="6ea57331-4519-4936-a19c-4865fcddc9c3" type="Person" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><gender>male</gender><country>GB</country><area id="8a754a16-0027-3a29-b6d7-2b40ea0481ed"><name>United Kingdom</name><sort-name>United Kingdom</sort-name></area><disambiguation>UK psytrance producer Richard Kegg</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="3af3e049-c350-49e4-b1f1-c890c6743acc" ext:score="61"><name>Purple</name><sort-name>Purple</sort-name><disambiguation>unknown</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="4779ae48-56fd-4956-b9f9-60ee885d9624" type="Group" ext:score="56"><name>Deep Purple Rain</name><sort-name>Deep Purple Rain</sort-name><area id="f265b80c-e446-440f-b636-16ba184bf4f4"><name>Grand Junction</name><sort-name>Grand Junction</sort-name></area><life-span><ended>false</ended></life-span></artist><artist id="ef3738cf-1871-46fa-81ba-4ed617f22119" type="Group" ext:score="56"><name>Deep Purple in Rock</name><sort-name>Deep Purple in Rock</sort-name><disambiguation>North East England</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="deba1b8c-1dc6-4326-bb47-634ce0cd0f18" ext:score="40"><name>Purple Buddah</name><sort-name>Purple Buddah</sort-name><life-span><ended>false</ended></life-span><alias-list><alias sort-name="Purple Buddha">Purple Buddha</alias></alias-list></artist><artist id="b4990264-2909-47b3-936d-f81553fbd69f" type="Person" ext:score="40"><name>DarK PurPLe</name><sort-name>PurPLe, DarK</sort-name><gender>male</gender><life-span><ended>false</ended></life-span></artist><artist id="1fb9df76-800b-4fa3-9dae-3a95fe1f5339" type="Group" ext:score="40"><name>Purple Haze</name><sort-name>Purple Haze</sort-name><country>US</country><area id="489ce91b-6658-3307-9877-795b68554c98"><name>United States</name><sort-name>United States</sort-name></area><disambiguation>American a cappella group</disambiguation><life-span><begin>1996</begin><ended>false</ended></life-span><alias-list><alias sort-name="Northwestern University Purple Haze">Northwestern University Purple Haze</alias></alias-list></artist><artist id="c96ee383-7ba9-4a9c-9843-7496469328c5" type="Person" ext:score="39"><name>Purple Chapel</name><sort-name>Chapel, Purple</sort-name><gender>male</gender><country>GB</country><area id="8a754a16-0027-3a29-b6d7-2b40ea0481ed"><name>United Kingdom</name><sort-name>United Kingdom</sort-name></area><begin-area id="1e23d84b-202e-3fc8-8c49-debc71e9eb16"><name>Nigeria</name><sort-name>Nigeria</sort-name></begin-area><life-span><begin>1990-02-28</begin><ended>false</ended></life-span></artist><artist id="9f71a1fc-d044-49cc-8301-f0e883f9715d" type="Person" ext:score="39"><name>Purple Haze</name><sort-name>Purple Haze</sort-name><gender>male</gender><country>NL</country><area id="ef1b7cc0-cd26-36f4-8ea0-04d9623786c7"><name>Netherlands</name><sort-name>Netherlands</sort-name></area><disambiguation>trance artist Sander Ketelaars a.k.a. Sander van Doorn</disambiguation><life-span><ended>false</ended></life-span><alias-list><alias sort-name="Sander Van Doorn presents Purple Haze" type="Search hint">Sander Van Doorn presents Purple Haze</alias><alias sort-name="Sander van Doorn Pres. Purple Haze" type="Search hint">Sander van Doorn Pres. Purple Haze</alias></alias-list></artist><artist id="ea256848-006c-4d76-bd42-c46ff5fec4a0" type="Person" ext:score="39"><name>MC Purple</name><sort-name>Purple, MC</sort-name><disambiguation>Renegade Boys</disambiguation><life-span><ended>false</ended></life-span><alias-list><alias sort-name="Purple">Purple</alias></alias-list></artist><artist id="81636b1d-ff23-4d6b-883a-d45c9eab49ef" type="Person" ext:score="39"><name>David Purple</name><sort-name>Purple, David</sort-name><gender>male</gender><disambiguation>Engineer</disambiguation><life-span><ended>false</ended></life-span><alias-list><alias sort-name="Purple, Dave">Dave Purple</alias></alias-list></artist><artist id="8edd574a-9279-4ffe-80d9-ed8b74759452" type="Group" ext:score="38"><name>Purple Project</name><sort-name>Purple Project</sort-name><country>JP</country><area id="2db42837-c832-3c27-b4a3-08198f75693c"><name>Japan</name><sort-name>Japan</sort-name></area><disambiguation>Hideki Matsutake, Konae Imafuji, Masashi Komatsubara</disambiguation><life-span><ended>false</ended></life-span></artist><artist id="d192782d-aeb6-4192-9d51-c49826821462" type="Group" ext:score="38"><name>Purple Pilgrims</name><sort-name>Purple Pilgrims</sort-name><country>NZ</country><area id="8524c7d9-f472-3890-a458-f28d5081d9c4"><name>New Zealand</name><sort-name>New Zealand</sort-name></area><life-span><ended>false</ended></life-span></artist><artist id="101047fb-521d-404f-9115-30223e486ddd" type="Group" ext:score="38"><name>Purple Popcorn</name><sort-name>Purple Popcorn</sort-name><begin-area id="4a9aeb42-3763-4234-8fb8-1167ac1dfdfe"><name>Miami</name><sort-name>Miami</sort-name></begin-area><life-span><ended>false</ended></life-span></artist><artist id="532ec01e-c18a-4e62-b3d2-f8e6de2ad846" ext:score="38"><name>PURPLE REVEL</name><sort-name>PURPLE REVEL</sort-name><life-span><ended>false</ended></life-span><tag-list><tag count="1"><name>likedis auto</name></tag></tag-list></artist><artist id="7cb676cf-92e9-4943-8ae5-fc7e6198a8d5" ext:score="38"><name>Purple Pawz</name><sort-name>Purple Pawz</sort-name><life-span><ended>false</ended></life-span></artist><artist id="a11c61a7-5764-42b2-8141-8f16c9212bed" ext:score="38"><name>Purple Powder</name><sort-name>Purple Powder</sort-name><life-span><ended>false</ended></life-span></artist><artist id="a2b0feb8-9214-480b-a6a1-db25c6a3d9a4" ext:score="38"><name>Miss Purple</name><sort-name>Purple, Miss</sort-name><life-span><ended>false</ended></life-span></artist></artist-list></metadata>)xml";

    EXPECT_EQ( "79491354-3d83-40e3-9d8e-7592d58d790a", ModAlbums::mbid_parse( body ) );
}

TEST(MusicbrainzTest, parse_artist_metadata) {

    const char* body = R"xml(
                       <?xml version="1.0" encoding="UTF-8"?><metadata xmlns="http://musicbrainz.org/ns/mmd-2.0#"><artist type="Person" type-id="b6e035f4-3ce9-331c-97df-83397230b0df" id="f27ec8db-af05-4f36-916e-3d57f91ecf5e"><name>Michael Jackson</name><sort-name>Jackson, Michael</sort-name><disambiguation>“King of Pop”</disambiguation><ipi>00002961801</ipi><ipi-list><ipi>00002961801</ipi><ipi>00523553664</ipi></ipi-list><isni-list><isni>000000011023081X</isni></isni-list><gender id="36d3d30a-839d-3eda-8cb3-29be4384e4a9">Male</gender><country>US</country><area id="489ce91b-6658-3307-9877-795b68554c98"><name>United States</name><sort-name>United States</sort-name><iso-3166-1-code-list><iso-3166-1-code>US</iso-3166-1-code></iso-3166-1-code-list></area><begin-area id="34357067-8f7f-4c7a-8d5e-99b6e60f7891"><name>Gary</name><sort-name>Gary</sort-name></begin-area><end-area id="1f40c6e1-47ba-4e35-996f-fe6ee5840e62"><name>Los Angeles</name><sort-name>Los Angeles</sort-name></end-area><life-span><begin>1958-08-29</begin><end>2009-06-25</end><ended>true</ended></life-span><relation-list target-type="url"><relation type-id="e4d73442-3762-45a8-905c-401da65544ed" type="lyrics"><target id="33e92637-d047-4bc8-9447-9e1677d53b09">http://decoda.com/michael-jackson-lyrics</target></relation><relation type="lyrics" type-id="e4d73442-3762-45a8-905c-401da65544ed"><target id="d19b6ded-40d7-4d55-a8ec-9c426364dfd5">http://genius.com/artists/Michael-jackson</target></relation><relation type="lyrics" type-id="e4d73442-3762-45a8-905c-401da65544ed"><target id="0da939eb-c275-4e2a-8dcb-75d738aa7735">http://lyrics.wikia.com/Michael_Jackson</target></relation><relation type="other databases" type-id="d94fb61c-fa20-4e3c-a19a-71a949fb2c55"><target id="8aad8396-38bf-4ce5-8b14-f67e1cde0f41">http://musicmoz.org/Bands_and_Artists/J/Jackson,_Michael/</target></relation><relation type-id="e4d73442-3762-45a8-905c-401da65544ed" type="lyrics"><target id="fba3ee32-77ba-4f79-a178-97113ecf214a">http://muzikum.eu/en/122-3710/michael-jackson/lyrics.html</target></relation><relation type="other databases" type-id="d94fb61c-fa20-4e3c-a19a-71a949fb2c55"><target id="e15d8e14-9c86-49fb-8764-99f17cccd811">http://ocremix.org/artist/5385/michael-jackson</target></relation><relation type-id="769085a1-c2f7-4c24-a532-2375a77693bd" type="streaming music"><target id="1656cd65-7658-4945-a47d-f49e7138b461">http://open.spotify.com/artist/3fMbdgg4jU18AjLCKBhRSm</target></relation><relation type="vgmdb" type-id="0af15ab3-c615-46d6-b95b-a5fcd2a92ed9"><target id="c0e48fe8-dbe3-47fb-b3f1-34edb5b3f261">http://vgmdb.net/artist/7312</target></relation><relation type-id="e8571dcc-35d4-4e91-a577-a3382fd84460" type="VIAF"><target id="31f63e3e-84b7-4c58-a864-64bcaf54ff8b">http://viaf.org/viaf/27092134</target></relation><relation type="discography" type-id="4fb0eeec-a6eb-4ae3-ad52-b55765b94e8f"><target id="c4828803-d0fb-4e35-ae51-264e4912972d">http://weave.me/app/weave.html?Michael_Jackson</target></relation><relation type-id="6b3e3c85-0002-4f34-aca6-80ace0d7e846" type="allmusic"><target id="6d833c39-9b41-4fa9-a9f6-2ac7f4b971fb">http://www.allmusic.com/artist/mn0000467203</target></relation><relation type-id="d028a975-000c-4525-9333-d3c8425e4b54" type="BBC Music page"><target id="0ad9cef2-a1a9-401c-bd02-a673dd95cb15">http://www.bbc.co.uk/music/artists/f27ec8db-af05-4f36-916e-3d57f91ecf5e</target></relation><relation type-id="94c8b0cc-4477-4106-932c-da60e63de61c" type="IMDb"><target id="fb1e53a6-5b7a-42d6-b4fc-76306fb0d8de">http://www.imdb.com/name/nm0001391/</target></relation><relation type-id="35b3a50f-bf0e-4309-a3b4-58eeed8cee6a" type="online community"><target id="1c3ca1d3-4205-435e-b2ac-6c7d2908d70f">http://www.imeem.com/michaeljackson</target></relation><relation type-id="08db8098-c0df-4b78-82c3-c8697b4bba7f" type="last.fm"><target id="02ca5284-1024-47c1-93e0-481003a01be1">http://www.last.fm/music/Michael+Jackson</target></relation><relation type="official homepage" type-id="fe33d22f-c3b0-4d68-bd53-a856badf2b15"><target id="f03c74df-58fb-42b2-8974-d8ea74ad4beb">http://www.michaeljackson.com/</target></relation><relation type-id="f484f897-81cc-406e-96f9-cd799a04ee24" type="fanpage"><target id="1be5af05-e598-4c6c-b3c4-b5b794d3421f">http://www.mjfanclub.net/</target></relation><relation type-id="79c5b84d-a206-4f4c-9832-78c028c312c3" type="secondhandsongs"><target id="5cd63d78-1153-4418-95d5-04bde9ace5f2">http://www.secondhandsongs.com/artist/254</target></relation><relation type="official homepage" type-id="fe33d22f-c3b0-4d68-bd53-a856badf2b15"><target id="637cd810-10a3-4b37-bae7-78aae0be7ec8">http://www.sonymusic.co.jp/Music/International/Arch/ES/MichaelJackson/</target></relation><relation type-id="221132e9-e30e-43f2-a741-15afc4c5fa7c" type="image"><target id="4cac77c7-348e-4618-b550-4cc735cab638">https://commons.wikimedia.org/wiki/File:Michael_Jackson_in_1988.jpg</target></relation><relation type-id="29651736-fa6d-48e4-aadc-a557c6add1cb" type="wikipedia"><target id="a244980c-f333-40fb-bcf0-4b272a4bf6d8">https://en.wikipedia.org/wiki/Michael_Jackson</target></relation><relation type-id="bac47923-ecde-4b59-822e-d08f0cd10156" type="myspace"><target id="6de0c807-02ba-483a-a7bc-4a66662997af">https://myspace.com/michaeljackson</target></relation><relation type="social network" type-id="99429741-f3f6-484b-84f8-23af51991770"><target id="bb29981f-cf17-461f-88d3-365800f3cd07">https://plus.google.com/+MichaelJackson/</target></relation><relation type-id="d94fb61c-fa20-4e3c-a19a-71a949fb2c55" type="other databases"><target id="b73f0c6a-54d2-4c0f-b8a9-bd0625b998a6">https://rateyourmusic.com/artist/michael_jackson</target></relation><relation type="social network" type-id="99429741-f3f6-484b-84f8-23af51991770"><target id="1486d902-606c-4570-ad7f-5cf184914f5a">https://twitter.com/michaeljackson</target></relation><relation type="discogs" type-id="04a5b104-a4c2-4bac-99a1-7b837c37d9e4"><target id="01d7a3f4-00f9-4343-8043-cc518c73cc7f">https://www.discogs.com/artist/15885</target></relation><relation type-id="99429741-f3f6-484b-84f8-23af51991770" type="social network"><target id="7eb52fdf-8784-4c8b-83a9-e11b6e2c353a">https://www.facebook.com/michaeljackson</target></relation><relation type-id="689870a4-a1e4-4912-b17f-7b2664215698" type="wikidata"><target id="27011c1f-bd4f-474c-ae04-23e00f79ccd9">https://www.wikidata.org/wiki/Q2831</target></relation><relation type="youtube" type-id="6a540e5b-58c6-4192-b6ba-dbc71ec8fcf0"><target id="65cf3ee3-3070-4bda-8f75-eaa8f1f42031">https://www.youtube.com/user/michaeljackson</target></relation></relation-list></artist></metadata>)xml";

    auto _relations = ModAlbums::get_artist_metadata( body );
    EXPECT_EQ( 6U, _relations.size() );
    EXPECT_EQ( "http://www.allmusic.com/artist/mn0000467203", _relations["allmusic"] );
    EXPECT_EQ( "http://www.imdb.com/name/nm0001391/", _relations["IMDb"] );
    EXPECT_EQ( "http://www.sonymusic.co.jp/Music/International/Arch/ES/MichaelJackson/", _relations["official homepage"] );
    EXPECT_EQ( "https://commons.wikimedia.org/wiki/File:Michael_Jackson_in_1988.jpg", _relations["image"] );
    EXPECT_EQ( "https://en.wikipedia.org/wiki/Michael_Jackson", _relations["wikipedia"] );
    EXPECT_EQ( "https://www.discogs.com/artist/15885", _relations["discogs"] );
}
}//namespace mod
}//namespace cds
