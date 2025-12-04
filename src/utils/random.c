#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stddef.h>

char* luarmor_random_string(size_t length) {
    const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t chars_len = sizeof(chars) - 1;
    
    char* result = (char*)malloc((length + 1) * sizeof(char));
    if (!result) return NULL;
    
    static int seeded = 0;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = 1;
    }
    
    for (size_t i = 0; i < length; i++) {
        result[i] = chars[rand() % chars_len];
    }
    result[length] = '\0';
    
    return result;
}

