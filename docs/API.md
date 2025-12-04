# Luarmor C Library API Documentation

## Overview

The Luarmor C library provides a complete interface for validating keys through the Luarmor external key check API. The library handles all aspects of the API interaction including signature generation, HTTP communication, and response verification.

## Architecture

The library is organized into modular components:

- **Core Library** (`luarmor.c`) - Main API functions
- **SHA1 Utility** (`utils/sha1.c`) - Cryptographic hashing
- **Random Utility** (`utils/random.c`) - Nonce generation
- **HTTP Client** (`utils/http.c`) - Network communication
- **JSON Parser** (`utils/json_parser.c`) - Response parsing

## Data Structures

### luarmor_sync_info_t

Stores server synchronization information.

```c
typedef struct {
    int32_t server_time;    // UNIX timestamp from server
    char** nodes;            // Array of node URLs
    size_t node_count;       // Number of available nodes
} luarmor_sync_info_t;
```

**Memory Management:**
- Allocated by `luarmor_fetch_sync_info()`
- Freed by `luarmor_free_sync_info()`
- `nodes` array contains dynamically allocated strings

### luarmor_key_response_t

Contains the result of a key validation request.

```c
typedef struct {
    char* code;             // Response code (e.g., "KEY_VALID")
    char* message;           // Human-readable message
    char* signature;         // Server signature (if KEY_VALID)
    bool has_signature;      // Whether signature is present
    int32_t auth_expire;     // Expiration timestamp
    int32_t total_executions;// Total execution count
    char* note;              // Additional note from server
} luarmor_key_response_t;
```

**Memory Management:**
- Allocated by `luarmor_check_key()`
- Freed by `luarmor_free_key_response()`
- All string fields are dynamically allocated

### luarmor_config_t

Configuration required for key validation.

```c
typedef struct {
    char* secret_n1;         // First shared secret
    char* secret_n2;         // Second shared secret
    char* secret_n3;         // Third shared secret
    char* app_name;          // Application/integration name
    char* hwid;              // Hardware ID string
    char* executor_name;     // Executor/platform name
} luarmor_config_t;
```

**Note:** All fields must be non-NULL when passed to `luarmor_check_key()`.

### luarmor_error_t

Error codes returned by library functions.

```c
typedef enum {
    LUARMOR_OK = 0,                      // Success
    LUARMOR_ERROR_NETWORK = -1,          // Network/HTTP error
    LUARMOR_ERROR_JSON = -2,             // JSON parsing error
    LUARMOR_ERROR_INVALID_RESPONSE = -3, // Invalid response format
    LUARMOR_ERROR_SIGNATURE_MISMATCH = -4, // Signature verification failed
    LUARMOR_ERROR_MEMORY = -5            // Memory allocation failure
} luarmor_error_t;
```

## Function Reference

### luarmor_fetch_sync_info

```c
luarmor_error_t luarmor_fetch_sync_info(luarmor_sync_info_t* info);
```

Fetches server synchronization information from the Luarmor public sync endpoint.

**Process:**
1. Makes HTTP GET request to `https://sdkapi-public.luarmor.net/sync`
2. Parses JSON response containing server time and node list
3. Populates the `info` structure

**Parameters:**
- `info` - Pointer to `luarmor_sync_info_t` to populate (must not be NULL)

**Returns:**
- `LUARMOR_OK` - Successfully fetched and parsed sync info
- `LUARMOR_ERROR_NETWORK` - Failed to connect or receive response
- `LUARMOR_ERROR_JSON` - Response was not valid JSON or missing required fields
- `LUARMOR_ERROR_INVALID_RESPONSE` - NULL pointer passed

**Example:**
```c
luarmor_sync_info_t sync_info = {0};
luarmor_error_t result = luarmor_fetch_sync_info(&sync_info);
if (result == LUARMOR_OK) {
    printf("Server time: %d\n", sync_info.server_time);
    printf("Available nodes: %zu\n", sync_info.node_count);
}
```

### luarmor_free_sync_info

```c
void luarmor_free_sync_info(luarmor_sync_info_t* info);
```

Frees all memory allocated by `luarmor_fetch_sync_info()`.

**Parameters:**
- `info` - Pointer to `luarmor_sync_info_t` to free (safe to pass NULL)

**Note:** After calling this function, the structure should not be used unless `luarmor_fetch_sync_info()` is called again.

### luarmor_check_key

```c
luarmor_error_t luarmor_check_key(
    const luarmor_config_t* config,
    const char* key,
    const luarmor_sync_info_t* sync_info,
    luarmor_key_response_t* response
);
```

Validates a key through the Luarmor API with automatic signature generation and verification.

**Process:**
1. Randomly selects a node from `sync_info->nodes`
2. Generates a 16-character random nonce
3. Calculates request signature using secrets, key, server time, and HWID
4. Makes HTTP GET request to selected node with proper headers
5. Parses JSON response
6. Verifies response signature if code is "KEY_VALID"
7. Populates the `response` structure

**Signature Calculation:**
```
SHA1(client_nonce + secret_n1 + key + secret_n2 + server_time + secret_n3 + hwid)
```

**Response Signature Verification:**
```
SHA1(client_nonce + secret_n3 + "KEY_VALID")
```

**Parameters:**
- `config` - Configuration structure with secrets and identifiers (must not be NULL)
- `key` - 32-character alphanumeric key to validate (must not be NULL)
- `sync_info` - Sync information from `luarmor_fetch_sync_info()` (must not be NULL)
- `response` - Pointer to `luarmor_key_response_t` to populate (must not be NULL)

**Returns:**
- `LUARMOR_OK` - Request completed successfully (check `response->code` for validation result)
- `LUARMOR_ERROR_NETWORK` - Failed to connect or receive response
- `LUARMOR_ERROR_JSON` - Response was not valid JSON
- `LUARMOR_ERROR_SIGNATURE_MISMATCH` - Response signature verification failed (possible tampering)
- `LUARMOR_ERROR_MEMORY` - Memory allocation failure
- `LUARMOR_ERROR_INVALID_RESPONSE` - Invalid parameters or empty node list

**Response Codes:**
The `response->code` field may contain:
- `"KEY_VALID"` - Key is valid and active
- `"KEY_INVALID"` - Key does not exist
- `"KEY_EXPIRED"` - Key has expired
- `"KEY_BANNED"` - Key has been banned
- `"KEY_HWID_MISMATCH"` - Key is bound to different HWID
- Other error codes as defined by the API

**Example:**
```c
luarmor_config_t config = {
    .secret_n1 = "secret1",
    .secret_n2 = "secret2",
    .secret_n3 = "secret3",
    .app_name = "myapp",
    .hwid = "hwid-12345",
    .executor_name = "myexec"
};

luarmor_key_response_t response = {0};
luarmor_error_t result = luarmor_check_key(&config, "USER_KEY_32_CHARS", &sync_info, &response);

if (result == LUARMOR_OK) {
    if (luarmor_is_key_valid(&response)) {
        printf("Key valid until: %d\n", response.auth_expire);
    } else {
        printf("Key invalid: %s\n", response.message);
    }
    luarmor_free_key_response(&response);
}
```

### luarmor_free_key_response

```c
void luarmor_free_key_response(luarmor_key_response_t* response);
```

Frees all memory allocated by `luarmor_check_key()`.

**Parameters:**
- `response` - Pointer to `luarmor_key_response_t` to free (safe to pass NULL)

### luarmor_is_key_valid

```c
bool luarmor_is_key_valid(const luarmor_key_response_t* response);
```

Convenience function to check if a key response indicates a valid key.

**Parameters:**
- `response` - Pointer to `luarmor_key_response_t` (may be NULL)

**Returns:**
- `true` if `response->code` equals "KEY_VALID"
- `false` otherwise (including NULL response)

**Example:**
```c
if (luarmor_is_key_valid(&response)) {
    printf("Key is valid!\n");
} else {
    printf("Key validation failed\n");
}
```

### luarmor_random_string

```c
char* luarmor_random_string(size_t length);
```

Generates a random alphanumeric string of specified length.

**Parameters:**
- `length` - Desired string length

**Returns:**
- Pointer to allocated string containing random characters (a-z, A-Z, 0-9)
- `NULL` on memory allocation failure

**Memory:** Caller must free the returned string with `free()`.

**Example:**
```c
char* nonce = luarmor_random_string(16);
if (nonce) {
    printf("Nonce: %s\n", nonce);
    free(nonce);
}
```

### luarmor_sha1_hash

```c
char* luarmor_sha1_hash(const char* data);
```

Computes SHA1 hash of input data, returning lowercase hexadecimal representation.

**Parameters:**
- `data` - Input string to hash (must not be NULL)

**Returns:**
- Pointer to 40-character hexadecimal string (lowercase)
- `NULL` on failure

**Memory:** Caller must free the returned string with `free()`.

**Example:**
```c
char* hash = luarmor_sha1_hash("test");
if (hash) {
    printf("SHA1: %s\n", hash);
    free(hash);
}
```

## Usage Patterns

### Complete Key Validation Flow

```c
luarmor_sync_info_t sync_info = {0};
luarmor_error_t result = luarmor_fetch_sync_info(&sync_info);
if (result != LUARMOR_OK) {
    fprintf(stderr, "Failed to fetch sync info\n");
    return 1;
}

luarmor_config_t config = {
    .secret_n1 = get_secret_n1(),
    .secret_n2 = get_secret_n2(),
    .secret_n3 = get_secret_n3(),
    .app_name = "myapp",
    .hwid = get_hwid(),
    .executor_name = "myexec"
};

luarmor_key_response_t response = {0};
result = luarmor_check_key(&config, user_key, &sync_info, &response);

if (result == LUARMOR_OK) {
    if (luarmor_is_key_valid(&response)) {
        printf("Key validated successfully\n");
        printf("Expires: %d\n", response.auth_expire);
    } else {
        printf("Key validation failed: %s\n", response.message);
    }
    luarmor_free_key_response(&response);
} else {
    fprintf(stderr, "Key check error: %d\n", result);
}

luarmor_free_sync_info(&sync_info);
```

### Reusing Sync Info

Sync info can be reused for multiple key checks:

```c
luarmor_sync_info_t sync_info = {0};
luarmor_fetch_sync_info(&sync_info);

for (int i = 0; i < key_count; i++) {
    luarmor_key_response_t response = {0};
    luarmor_check_key(&config, keys[i], &sync_info, &response);
    luarmor_free_key_response(&response);
}

luarmor_free_sync_info(&sync_info);
```

## Error Handling Best Practices

1. Always check return values
2. Free allocated structures even on error
3. Handle network errors gracefully (retry logic)
4. Verify signatures are checked for KEY_VALID responses
5. Use `luarmor_is_key_valid()` for convenience, but also check `response->code` for specific error types

## Thread Safety

- HTTP operations are thread-safe (libcurl handles this)
- Random number generation uses static state (not thread-safe)
- For multi-threaded use, protect `luarmor_random_string()` calls or use thread-local storage

## Performance Considerations

- Sync info can be cached and reused (server time may drift slightly)
- Each key check makes one HTTP request
- Signature calculation is fast (SHA1)
- JSON parsing overhead is minimal

## Security Notes

- Never log or expose shared secrets
- Verify response signatures to prevent tampering
- Use HTTPS (enforced by library)
- Protect HWID generation to prevent spoofing
- Obfuscate authentication code in production binaries

