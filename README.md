[![Build Status](https://travis-ci.org/squawkcpp/cds.svg?branch=master)](https://travis-ci.org/squawkcpp/cds)

# Content Directory Server (CDS)

## about

the squawk content directory server (CDS) serves mediafiles from the local storage. the mediafile are enriched with local and remote metadata.

## install the server:

### install binary package

### use docker image

### install from source

To install the server from source you need cmake and a recent c++ (C++14) compiler installed. The developmeent packages of the
dependencies must be available to the compiler.

##### install dependencies on ubuntu

##### get the source

##### compile

##### install

## usage:

start the content directory server from the command line:

```
cds [OPTION...]
```

##### options:

name | value | description
------------ | ------------- | -------------
--directory|PATH|path to the directory with the mediafiles. Multiple entries will result in a list containing all directories.
--listen|IP|API Webserver IP-Adress to bind to.
--http-port|PORT|API Webserver IP Port to bind to.
--tmp-directory|PATH|temporary folder for the thumbnails.
--tmdb-key|KEY|API key for tmdb.
--amazon-access-key|KEY|Access key for amazon.
--amazon-key|KEY|API key for amazon.
--redis|HOST|Redis Database (default: localhost) (default: localhost)
--redis-port|PORT|Redis Database port (default: 6379) (default: 6379)
--help| |Print help

## api:

### Base URL

all the requests have the base url of your server:

```http://<SERVER_IP>:<PORT>```

### access the directory

the directory is structured as a tree. the content can be traversed from the **`root`** node.
beside of the class specific attributes, the node will always contain the **`key`, `parent`** and **`cls`** attributes.

#### load the child elements of a node.

```
GET /{node}/nodes
```

##### parameters:

name | value | description
------------ | ------------- | -------------
node | {key} | select from which parent node the list is created. root, for the keys **file**, **ebook**, **movie**, **album**, **serie**, **artist** and **image** the type collection is returned.

##### query paremeters:

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

#### rescan the database.

```
GET rescan
```

##### query paremeters:

 name | value | description
 ------------ | ------------- | -------------
 flush | true, false | flush the database before import the files.

## remote metadata providers:

for information retrievel the following database services can be used:

- [amazon]()
- [tmdb]()

## dependencies:

- [asio](http://think-async.com)
- [cxxopts](https://github.com/jarro2783/cxxopts)
- [format::fmt](https://github.com/fmtlib/fmt)
- [rapidjson](https://github.com/miloyip/rapidjson)
- [RapidXML NS](https://github.com/svgpp/rapidxml_ns)
- [re2](https://github.com/google/re2) [redox](https://github.com/hmartiro/redox)
- [spdlog](https://github.com/gabime/spdlog)

## licence:
