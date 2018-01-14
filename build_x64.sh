#!/bin/bash

set -e

sudo docker run -it -v $(pwd):/repo -v $(pwd)/.build:/build toolchain /bin/bash -c 'cd /build && \
	conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan && \
	conan remote add squawkcpp https://api.bintray.com/conan/squawkcpp/Re2 && \
	conan install /repo --build=missing -s compiler=gcc -s compiler.libcxx=libstdc++11 && \
	cmake -H/repo -B/build -G Ninja -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -Dbuild_tests=on -Dbuild_example_server=on -Dbuild_documentation=on && \
	cmake --build /build && \
	cmake --build /build --target test && \
	cmake --build /build --target doc && \
	cmake --build /build --target package'

#$BUILD /bin/bash -c 'cmake -H/repo -B/build -G Ninja -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -Dbuild_tests=on -Dbuild_example_server=on -Dbuild_documentation=on'
#$BUILD /bin/bash -c 'cmake --build /build'
#$BUILD /bin/bash -c 'cmake --build /build --target test'
#$BUILD /bin/bash -c 'cmake --build /build --target doc'
#$BUILD /bin/bash -c 'cmake --build /build --target package'


sudo docker build -f docker/Dockerfile -t lightning .

sudo rm -rf .build
