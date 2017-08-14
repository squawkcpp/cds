FROM        ubuntu:xenial
CMD          bash

# Required system packages
RUN apt-get update && apt-get -y install git cmake ninja-build graphviz mscgen doxygen astyle cppcheck \
        valgrind clang make libsdl2-dev libboost-all-dev libao-dev libimlib2-dev libsqlite3-dev libxml++-dev libpcre++-dev \
	libxml2-dev libmp3lame-dev libflac++-dev libpoppler-cpp-dev libcurl4-openssl-dev libcurlpp-dev libzmq-dev libsigc++-2.0-dev \
	libssl-dev libarchive-dev uuid-dev libhiredis-dev libev-dev libmagic-dev libopencv-dev libmusicbrainz5-dev \
	libavcodec-dev libavdevice-dev libavfilter-dev libavformat-dev libavresample-dev libavcodec-extra	
	
