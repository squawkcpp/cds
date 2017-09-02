[![Build Status](https://travis-ci.org/squawkcpp/cds.svg?branch=master)](https://travis-ci.org/squawkcpp/cds)

# Content Directory Server (CDS)

## about:

the squawk content directory server (CDS) serves mediafiles from the local storage. the mediafile are enriched with local and remote metadata.

## usage:



## restful api:

The REST API allows you to query the content of the directory server.

### Base URL

all the requests have the base url of your server:

```http://<SERVER_IP>:<PORT>```

### access the directory

#### node list
**<code>GET</code> /[root|file|ebook|movie|album|serie|artist|image|{digit}]/nodes**
#####Parameters:
name | value | description
------------ | ------------- | -------------
sort | alpha, timestamp | sort list alphanumerical or by last access timestamp.
order | asc, desc | sort ascending or descending. default ist ascending.
filter | tag | filter results by tag.
index | {digit} | start index of the result list.
count | {digit} | number of results in the result list.

#### node
**<code>GET</code> /{digit}**

#### keywords
**<code>GET</code> /[ebook|movie|album|serie|artist|image]/name|{keyword}**

### manage the server

- **[<code>GET</code> config](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)**
- **[<code>GET</code> status](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)**

## remote metadata providers:

for information retrievel the following database services can be used:
[amazon]()
[tmdb]()

## dependencies:

[asio](http://think-async.com) [cxxopts](https://github.com/jarro2783/cxxopts) [format::fmt](https://github.com/fmtlib/fmt)
[rapidjson](https://github.com/miloyip/rapidjson) [RapidXML NS](https://github.com/svgpp/rapidxml_ns)
[re2](https://github.com/google/re2) [redox](https://github.com/hmartiro/redox) [spdlog](https://github.com/gabime/spdlog)

## licence:
