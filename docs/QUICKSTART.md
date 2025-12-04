# Quick Start Guide

Get up and running with the Luarmor C library in minutes.

## 1. Install Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libcurl4-openssl-dev libssl-dev libcjson-dev cmake build-essential
```

**Arch Linux:**
```bash
sudo pacman -S curl openssl cjson cmake gcc
```

**macOS:**
```bash
brew install curl openssl cjson cmake
```

## 2. Build the Library

```bash
git clone <repository-url>
cd Luarmor
mkdir build && cd build
cmake ..
make
```

## 3. Create Your First Program

Create `example.c`:

```c
#include <luarmor.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    luarmor_sync_info_t sync_info = {0};
    
    if (luarmor_fetch_sync_info(&sync_info) != LUARMOR_OK) {
        fprintf(stderr, "Failed to fetch sync info\n");
        return 1;
    }
    
    luarmor_config_t config = {
        .secret_n1 = "your_secret_1",
        .secret_n2 = "your_secret_2",
        .secret_n3 = "your_secret_3",
        .app_name = "myapp",
        .hwid = "my-hwid-12345",
        .executor_name = "myexec"
    };
    
    luarmor_key_response_t response = {0};
    luarmor_error_t result = luarmor_check_key(
        &config,
        "YOUR_32_CHARACTER_KEY_HERE",
        &sync_info,
        &response
    );
    
    if (result == LUARMOR_OK) {
        if (luarmor_is_key_valid(&response)) {
            printf("✓ Key is valid!\n");
            printf("  Expires: %d\n", response.auth_expire);
        } else {
            printf("✗ Key invalid: %s\n", response.message);
        }
        luarmor_free_key_response(&response);
    } else {
        fprintf(stderr, "Error: %d\n", result);
    }
    
    luarmor_free_sync_info(&sync_info);
    return 0;
}
```

## 4. Compile and Run

```bash
gcc example.c -o example -L../build -lluarmor -lcurl -lssl -lcrypto -lcjson
./example
```

## 5. Next Steps

- Read the [API Documentation](API.md) for detailed function reference
- Check [Building Guide](BUILDING.md) for advanced build options
- Review the test suite in `tests/test_luarmor.c` for more examples

## Common Issues

**"Library not found" error:**
```bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:../build
```

Or use full path:
```bash
gcc example.c -o example -L$(pwd)/../build -lluarmor -lcurl -lssl -lcrypto -lcjson
```

**Missing headers:**
Install development packages (they have `-dev` suffix on Debian/Ubuntu).

**API credentials:**
Contact federal to get your shared secrets and app name approved.

