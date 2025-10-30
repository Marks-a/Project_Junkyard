#!/bin/bash
set -e


mkdir -p build
cd build

cmake -DCMAKE_TOOLCHAIN_FILE=/home/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .

./TestResultLightweightEngine
