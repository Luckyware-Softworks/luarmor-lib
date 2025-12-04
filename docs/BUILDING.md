# Building the Luarmor C Library

## Prerequisites

### Required Dependencies

- **C Compiler**: GCC 4.9+ or Clang 3.5+ with C11 support
- **CMake**: Version 3.10 or higher
- **libcurl**: Development headers and library
- **OpenSSL**: Development headers and library (for SHA1)
- **cJSON**: Development headers and library (for JSON parsing)

### System-Specific Installation

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libcurl4-openssl-dev libssl-dev libcjson-dev
```

#### Arch Linux
```bash
sudo pacman -S base-devel cmake curl openssl cjson
```

#### Fedora/RHEL/CentOS
```bash
sudo dnf install gcc cmake libcurl-devel openssl-devel libcjson-devel
```

#### macOS (Homebrew)
```bash
brew install cmake curl openssl cjson
```

#### macOS (MacPorts)
```bash
sudo port install cmake curl openssl cjson
```

## Build Process

### Standard Build

```bash
mkdir build
cd build
cmake ..
make
```

### Release Build (Optimized)

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Debug Build (With Symbols)

```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Verbose Build Output

```bash
cmake ..
make VERBOSE=1
```

## Build Output

After successful build, you'll find:

- `libluarmor.a` - Static library file
- `luarmor_test` - Test executable

## Installation

### System-Wide Installation

```bash
cd build
sudo make install
```

This installs:
- Library: `/usr/local/lib/libluarmor.a`
- Header: `/usr/local/include/luarmor.h`

### Custom Installation Prefix

```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/luarmor ..
make
sudo make install
```

## Using the Library

### With GCC/Clang

```bash
gcc your_app.c -o your_app -lluarmor -lcurl -lssl -lcrypto -lcjson
```

### With CMake

```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(CJSON REQUIRED libcjson)

add_executable(your_app your_app.c)
target_link_libraries(your_app PRIVATE luarmor ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${CJSON_LIBRARIES})
target_include_directories(your_app PRIVATE /usr/local/include)
```

### With pkg-config

If you install pkg-config files, you can use:

```bash
gcc your_app.c -o your_app $(pkg-config --cflags --libs luarmor)
```

## Testing

Run the test suite:

```bash
cd build
./luarmor_test
```

Or with CMake:

```bash
cd build
ctest
```

## Troubleshooting

### Missing Dependencies

If CMake fails to find dependencies:

```bash
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
cmake ..
```

### OpenSSL Not Found

```bash
cmake -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl ..
```

### cJSON Not Found

Install cJSON from source if needed:

```bash
git clone https://github.com/DaveGamble/cJSON.git
cd cJSON
mkdir build
cd build
cmake ..
make
sudo make install
```

### Linker Errors

Ensure all required libraries are linked:

```bash
gcc your_app.c -o your_app -L/usr/local/lib -lluarmor -lcurl -lssl -lcrypto -lcjson
```

### Architecture Mismatch

For 64-bit builds on 32-bit systems:

```bash
cmake -DCMAKE_C_FLAGS="-m64" ..
```

## Cross-Compilation

### For Windows (MinGW)

```bash
cmake -DCMAKE_SYSTEM_NAME=Windows \
      -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
      -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32 ..
make
```

### For ARM Linux

```bash
cmake -DCMAKE_SYSTEM_NAME=Linux \
      -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc \
      -DCMAKE_FIND_ROOT_PATH=/usr/arm-linux-gnueabihf ..
make
```

## Clean Build

To start fresh:

```bash
rm -rf build
mkdir build
cd build
cmake ..
make
```

## Build Options

CMake options can be set:

```bash
cmake -DOPTION_NAME=value ..
```

Available options (if added to CMakeLists.txt):
- `CMAKE_BUILD_TYPE`: Release, Debug, RelWithDebInfo, MinSizeRel
- `CMAKE_INSTALL_PREFIX`: Installation directory
- `BUILD_TESTING`: Enable/disable tests (ON/OFF)

## Static vs Dynamic Linking

The library is built as static by default. To build as shared:

Modify `CMakeLists.txt`:
```cmake
add_library(luarmor SHARED ...)
```

Then rebuild.

## Optimization Flags

For maximum performance:

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3 -march=native" ..
make
```

## Sanitizers

For debugging with AddressSanitizer:

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-fsanitize=address -g" ..
make
```

For debugging with Valgrind:

```bash
valgrind --leak-check=full ./luarmor_test
```

