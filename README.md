# Content Directory Server (CDS)

## about:

the squawk content directory server (CDS) serves mediafiles from the local storage. the mediafile are enriched with local and remote metadata.

## usage:



## restful api:

The REST API allows you to query the content of the directory server.

### Base URL

All URLs referenced in the documentation have the following base:

### access the directory

- **[<code>GET</code> nodes](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)** \[sort=alpha,timestamp|order=asc,desc|filter=keyword\]
- **[<code>GET</code> node](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)**
- **[<code>GET</code> nodes](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)**

### manage the server

- **[<code>GET</code> config](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)**
- **[<code>GET</code> status](https://github.com/500px/api-documentation/blob/master/endpoints/photo/GET_photos.md)**

http://<YOUR_SERVER_IP>:<PORT>

## remote metadata providers:

for information retrievel the following database services can be used:
[amazon]()
[tmdb]()

## dependencies:

[asio](http://think-async.com) [cxxopts](https://github.com/jarro2783/cxxopts) [format::fmt](https://github.com/fmtlib/fmt)
[rapidjson](https://github.com/miloyip/rapidjson) [RapidXML NS](https://github.com/svgpp/rapidxml_ns)
[re2](https://github.com/google/re2) [redox](https://github.com/hmartiro/redox) [spdlog](https://github.com/gabime/spdlog)

## licence:
