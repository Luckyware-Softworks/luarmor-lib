# Luarmor C Library

A modular C library for integrating with the Luarmor external key check API. This library provides a clean interface for validating keys through the Luarmor ad reward system.

## Features

- Fetch server sync information and node lists
- Validate keys with automatic signature generation and verification
- Modular architecture with separate utility modules
- Comprehensive test suite
- Memory-safe with proper cleanup functions
- Thread-safe HTTP client using libcurl

## Requirements

- C11 compiler (GCC, Clang, etc.)
- CMake 3.10 or higher
- libcurl (development headers)
- OpenSSL (development headers)
- cJSON (development headers)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libcurl4-openssl-dev libssl-dev libcjson-dev cmake build-essential
```

**Arch Linux:**
```bash
sudo pacman -S curl openssl cjson cmake gcc
```

**macOS (Homebrew):**
```bash
brew install curl openssl cjson cmake
```

## Building

```bash
mkdir build
cd build
cmake ..
make
```

This will create:
- `libluarmor.a` - Static library
- `luarmor_test` - Test executable

## Installation

After building, you can install the library system-wide:

```bash
sudo make install
```

Or copy the library and header manually:
```bash
sudo cp libluarmor.a /usr/local/lib/
sudo cp ../include/luarmor.h /usr/local/include/
```

## Usage

### Basic Example

```c
#include <luarmor.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    luarmor_sync_info_t sync_info = {0};
    luarmor_error_t result = luarmor_fetch_sync_info(&sync_info);
    
    if (result != LUARMOR_OK) {
        fprintf(stderr, "Failed to fetch sync info: %d\n", result);
        return 1;
    }
    
    luarmor_config_t config = {
        .secret_n1 = "your_secret_n1",
        .secret_n2 = "your_secret_n2",
        .secret_n3 = "your_secret_n3",
        .app_name = "yourapp",
        .hwid = "your-hwid-string",
        .executor_name = "yourexec"
    };
    
    luarmor_key_response_t response = {0};
    result = luarmor_check_key(&config, "USER_KEY_HERE", &sync_info, &response);
    
    if (result == LUARMOR_OK) {
        if (luarmor_is_key_valid(&response)) {
            printf("Key is valid!\n");
            printf("Expires: %d\n", response.auth_expire);
            printf("Executions: %d\n", response.total_executions);
        } else {
            printf("Key invalid: %s\n", response.message);
        }
        luarmor_free_key_response(&response);
    } else {
        fprintf(stderr, "Key check failed: %d\n", result);
    }
    
    luarmor_free_sync_info(&sync_info);
    return 0;
}
```

### Compiling Your Application

```bash
gcc your_app.c -o your_app -lluarmor -lcurl -lssl -lcrypto -lcjson
```

Or with CMake:
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(CJSON REQUIRED libcjson)

add_executable(your_app your_app.c)
target_link_libraries(your_app PRIVATE luarmor ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${CJSON_LIBRARIES})
```

## API Reference

### Types

#### `luarmor_sync_info_t`
Contains server time and available node URLs.
```c
typedef struct {
    int32_t server_time;
    char** nodes;
    size_t node_count;
} luarmor_sync_info_t;
```

#### `luarmor_key_response_t`
Contains the key validation response.
```c
typedef struct {
    char* code;
    char* message;
    char* signature;
    bool has_signature;
    int32_t auth_expire;
    int32_t total_executions;
    char* note;
} luarmor_key_response_t;
```

#### `luarmor_config_t`
Configuration for key checking.
```c
typedef struct {
    char* secret_n1;
    char* secret_n2;
    char* secret_n3;
    char* app_name;
    char* hwid;
    char* executor_name;
} luarmor_config_t;
```

#### `luarmor_error_t`
Error codes returned by library functions.
```c
typedef enum {
    LUARMOR_OK = 0,
    LUARMOR_ERROR_NETWORK = -1,
    LUARMOR_ERROR_JSON = -2,
    LUARMOR_ERROR_INVALID_RESPONSE = -3,
    LUARMOR_ERROR_SIGNATURE_MISMATCH = -4,
    LUARMOR_ERROR_MEMORY = -5
} luarmor_error_t;
```

### Functions

#### `luarmor_fetch_sync_info()`
Fetches server time and available node list from the Luarmor sync endpoint.

```c
luarmor_error_t luarmor_fetch_sync_info(luarmor_sync_info_t* info);
```

**Parameters:**
- `info` - Pointer to `luarmor_sync_info_t` structure to populate

**Returns:**
- `LUARMOR_OK` on success
- `LUARMOR_ERROR_NETWORK` on network failure
- `LUARMOR_ERROR_JSON` on JSON parsing failure

**Note:** Remember to call `luarmor_free_sync_info()` when done.

#### `luarmor_free_sync_info()`
Frees memory allocated by `luarmor_fetch_sync_info()`.

```c
void luarmor_free_sync_info(luarmor_sync_info_t* info);
```

#### `luarmor_check_key()`
Validates a key using the Luarmor API with automatic signature generation and verification.

```c
luarmor_error_t luarmor_check_key(
    const luarmor_config_t* config,
    const char* key,
    const luarmor_sync_info_t* sync_info,
    luarmor_key_response_t* response
);
```

**Parameters:**
- `config` - Configuration with secrets, app name, HWID, and executor name
- `key` - 32-character alphanumeric key to validate
- `sync_info` - Sync information from `luarmor_fetch_sync_info()`
- `response` - Pointer to `luarmor_key_response_t` to populate

**Returns:**
- `LUARMOR_OK` on success (check `response->code` for actual result)
- `LUARMOR_ERROR_NETWORK` on network failure
- `LUARMOR_ERROR_JSON` on JSON parsing failure
- `LUARMOR_ERROR_SIGNATURE_MISMATCH` if response signature verification fails
- `LUARMOR_ERROR_MEMORY` on memory allocation failure

**Note:** Remember to call `luarmor_free_key_response()` when done.

#### `luarmor_free_key_response()`
Frees memory allocated by `luarmor_check_key()`.

```c
void luarmor_free_key_response(luarmor_key_response_t* response);
```

#### `luarmor_is_key_valid()`
Checks if a key response indicates a valid key.

```c
bool luarmor_is_key_valid(const luarmor_key_response_t* response);
```

**Returns:**
- `true` if `response->code` is "KEY_VALID"
- `false` otherwise

#### `luarmor_random_string()`
Generates a random alphanumeric string of specified length.

```c
char* luarmor_random_string(size_t length);
```

**Returns:**
- Pointer to allocated string (caller must free)
- `NULL` on memory allocation failure

#### `luarmor_sha1_hash()`
Computes SHA1 hash of input data, returning lowercase hexadecimal string.

```c
char* luarmor_sha1_hash(const char* data);
```

**Returns:**
- Pointer to 40-character hex string (caller must free)
- `NULL` on failure

## Error Handling

Always check return values from library functions:

```c
luarmor_error_t result = luarmor_fetch_sync_info(&sync_info);
if (result != LUARMOR_OK) {
    switch (result) {
        case LUARMOR_ERROR_NETWORK:
            fprintf(stderr, "Network error\n");
            break;
        case LUARMOR_ERROR_JSON:
            fprintf(stderr, "JSON parsing error\n");
            break;
        default:
            fprintf(stderr, "Unknown error: %d\n", result);
    }
    return 1;
}
```

## Testing

Run the test suite:

```bash
cd build
./luarmor_test
```

Tests cover:
- Random string generation
- SHA1 hashing
- Signature calculation
- Sync info fetching
- Key validation
- Memory cleanup

## Thread Safety

The library uses libcurl which is thread-safe when properly initialized. However, the random number generator uses a static seed, so concurrent calls to `luarmor_random_string()` may not be fully thread-safe. For multi-threaded applications, consider using thread-local storage or mutex protection for random string generation.

## Memory Management

All functions that allocate memory require the caller to free it:
- `luarmor_fetch_sync_info()` - use `luarmor_free_sync_info()`
- `luarmor_check_key()` - use `luarmor_free_key_response()`
- `luarmor_random_string()` - use `free()`
- `luarmor_sha1_hash()` - use `free()`

## License

This library is provided as-is for integration with the Luarmor API.

## Documentation

- [Quick Start Guide](docs/QUICKSTART.md) - Get started in minutes
- [API Reference](docs/API.md) - Complete function documentation
- [Building Guide](docs/BUILDING.md) - Advanced build options and troubleshooting

## Luarmor API Documentation

For detailed information about the Luarmor API specification, see:
https://docs.luarmor.net/docs-for-3rd-parties/3rd-party-external-key-check-api

## Support

For questions about the Luarmor API, contact federal.

