#include <openssl/sha.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* luarmor_sha1_hash(const char* data) {
    if (!data) return NULL;
    
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)data, strlen(data), hash);
    
    char* hex = (char*)malloc((SHA_DIGEST_LENGTH * 2 + 1) * sizeof(char));
    if (!hex) return NULL;
    
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++) {
        sprintf(hex + (i * 2), "%02x", hash[i]);
    }
    hex[SHA_DIGEST_LENGTH * 2] = '\0';
    
    return hex;
}

