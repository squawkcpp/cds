[![Build Status](https://travis-ci.org/squawkcpp/cds.svg?branch=master)](https://travis-ci.org/squawkcpp/cds)

** this software is still under development, its not recommended to use it now. **

# Content Directory Server (CDS)

## About

the squawk content directory server (CDS) serves mediafiles from the local storage. the mediafile are enriched with local and remote metadata.
for information retrievel the following database services can be used:

- [amazon]()
- [tmdb]()


## Install and run

install on ubuntu:

<pre>
wget <release from github>
apt-get install -f <release file name>
apt-get upgrade
</pre>

start the content directory server from the command line:

```
cds [OPTION...]
```

##### options

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

### Docker image

a prebuild image is available on dockerhub. to install run the docker command with the
parameters for the cds server.

<pre>
sudo docker run -itd --link <REDIS> --name squawk-cds -v /srv:/srv:ro -p 9001:9001 \
    -e REDIS=<REDIS>
    -e AMAZON_ACCESS_KEY=<ACCESS_KEY>
    -e AMAZON_KEY=<KEY>
    -e TMDB_KEY=<KEY>
    -e DIRECTORY=/srv
    squawk/cds:VERSION
</pre>

the options are the same as in the command line. multiple directories can be listet separated by a comma.

## Api

all the requests have the base url of your server: `http://<SERVER_IP>:<PORT>`

the directory is structured as a tree. the content can be traversed from the **`root`** node.
beside of the class specific attributes, the node will always contain the **`key`, `parent`** and **`cls`** attributes.

#### **<code>GET</code> /{node}/nodes**
load the child elements of a node.

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

#### **<code>GET</code> /{node}/sort**
get the elements sort criterias.

#### **<code>GET</code> /{node}/path**
get path brief path elements for the node referenced by key.

#### **<code>GET</code> /{digit}**
load the node with the key

#### **<code>GET</code> /[ebook|movie|album|serie|artist|image]/name|{keyword}**

#### **<code>GET</code> /config**

#### **<code>GET</code> /status**

#### **<code>GET</code> /rescans**

##### query paremeters:

 name | value | description
 ------------ | ------------- | -------------
 flush | true, false | flush the database before import the files.

## Dependencies:

binary dependencies needs to be installed on the target system.


these libraries will be downloaded and staticaly linked during the compile process.

- [asio](http://think-async.com) is a cross-platform C++ library for network and low-level I/O programming that provides developers with a consistent asynchronous model using a modern C++ approach.
- [cxxopts](https://github.com/jarro2783/cxxopts), lightweight C++ command line option parser
- [format::fmt](http://fmtlib.net), small, safe and fast formatting library
- [rapidjson](http://rapidjson.org/), a fast JSON parser/generator for C++ with both SAX/DOM style API
- [RapidXML NS](https://github.com/svgpp/rapidxml_ns) library - RapidXML with added XML namespaces support
- [re2](https://github.com/google/re2) is a fast, safe, thread-friendly alternative to backtracking regular expression engines like those used in PCRE, Perl, and Python. It is a C++ library.
- [redox](https://github.com/hmartiro/redox), modern, asynchronous, and wicked fast C++11 client for Redis
- [spdlog](https://github.com/gabime/spdlog), super fast C++ logging library.

## Licence:

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
