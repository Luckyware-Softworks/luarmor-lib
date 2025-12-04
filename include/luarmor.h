#ifndef LUARMOR_H
#define LUARMOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    int32_t server_time;
    char** nodes;
    size_t node_count;
} luarmor_sync_info_t;

typedef struct {
    char* code;
    char* message;
    char* signature;
    bool has_signature;
    int32_t auth_expire;
    int32_t total_executions;
    char* note;
} luarmor_key_response_t;

typedef struct {
    char* secret_n1;
    char* secret_n2;
    char* secret_n3;
    char* app_name;
    char* hwid;
    char* executor_name;
} luarmor_config_t;

typedef enum {
    LUARMOR_OK = 0,
    LUARMOR_ERROR_NETWORK = -1,
    LUARMOR_ERROR_JSON = -2,
    LUARMOR_ERROR_INVALID_RESPONSE = -3,
    LUARMOR_ERROR_SIGNATURE_MISMATCH = -4,
    LUARMOR_ERROR_MEMORY = -5
} luarmor_error_t;

luarmor_error_t luarmor_fetch_sync_info(luarmor_sync_info_t* info);
void luarmor_free_sync_info(luarmor_sync_info_t* info);
luarmor_error_t luarmor_check_key(const luarmor_config_t* config, const char* key, const luarmor_sync_info_t* sync_info, luarmor_key_response_t* response);
void luarmor_free_key_response(luarmor_key_response_t* response);
bool luarmor_is_key_valid(const luarmor_key_response_t* response);
char* luarmor_random_string(size_t length);
char* luarmor_sha1_hash(const char* data);

#endif

