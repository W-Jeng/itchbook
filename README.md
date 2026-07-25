# Parsing of ITCH data

## Build
Debug build:

mkdir build && cd build
cmake -S .. -B . -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .

Release build (recommended for benchmarking):

mkdir build && cd build
cmake -S .. -B . -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .


