#!/bin/bash
set -e


mkdir -p build
cd build

cmake -DCMAKE_TOOLCHAIN_FILE=/home/vcpkg/scripts/buildsystems/vcpkg.cmake ..
cmake --build .

#./TestSDL2GLM
echo "Linux part output complete"


# Windows out
mkdir -p ./out

cmake -B build \
      -S .. \
      -DCMAKE_TOOLCHAIN_FILE=/home/vcpkg/scripts/buildsystems/vcpkg.cmake \
      -DVCPKG_TARGET_TRIPLET=x64-mingw-static \
      -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
      -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
      -DCMAKE_BUILD_TYPE=Release

cmake --build build --config Release

cp build/TestSDL2GLM.exe ./out/TestSDL2GLM.exe
echo 'Windows exe is created'

echo "Windows part output complete"
