#!/bin/bash

set -e

IMAGE=spielhuus/toolchain
TAG=`if [ -z "$1" ]; then echo "master"; else echo "$1" ; fi`
PID=$(sudo docker run -itd -v $(pwd):/repo -v $(pwd)/.build:/.build $IMAGE /bin/sh)
echo "build squawk-cds tag:$TAG, image:$PID"

DOCKER_EXEC="sudo docker exec $PID /bin/sh -c"

$DOCKER_EXEC "cd .build && conan install /repo --build=missing"
$DOCKER_EXEC "cmake -H/repo -B/.build -G Ninja -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -DCDS_TAG_VERSION=$TAG"
$DOCKER_EXEC "cmake --build /.build"
$DOCKER_EXEC "cmake --build /.build --target package"

sudo docker build -f docker/Dockerfile --build-arg CDS_TAG_VERSION=$TAG -t squawk-cds .

sudo docker rm -f $PID

