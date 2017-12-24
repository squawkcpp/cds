#!/bin/bash

sudo docker pull spielhuus/clang:latest
sudo docker run -itd --name build_cds -v $(pwd):/repo -v $(pwd)/build:/build spielhuus/clang
sudo docker exec build_cds /usr/sbin/pacman --noconfirm -S boost poppler opencv doxygen hiredis libev hdf5 make git

sudo docker exec build_cds cmake -H/repo -B/build -G Ninja -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -DCDS_TAG_VERSION=master
sudo docker exec build_cds cmake --build /build
sudo docker exec build_cds /build/test_cds
sudo docker exec build_cds cmake --build /build --target package

sudo docker build -f docker/Dockerfile --build-arg CDS_TAG_VERSION=master -t cds .

sudo docker rm -f build_cds
sudo rm -rf build


