#include <stddef.h>
#include "../include/luarmor.h"
#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "utils/sha1.c"
#include "utils/random.c"
#include "utils/http.c"
#include "utils/json_parser.c"

char* luarmor_http_get(const char* url, struct curl_slist* headers);
int luarmor_parse_sync_info(const char* json_str, luarmor_sync_info_t* info);
int luarmor_parse_key_response(const char* json_str, luarmor_key_response_t* response);

luarmor_error_t luarmor_fetch_sync_info(luarmor_sync_info_t* info) {
    if (!info) return LUARMOR_ERROR_INVALID_RESPONSE;
    
    static int curl_initialized = 0;
    if (!curl_initialized) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_initialized = 1;
    }
    
    const char* url = "https://sdkapi-public.luarmor.net/sync";
    char* response = luarmor_http_get(url, NULL);
    
    if (!response) {
        return LUARMOR_ERROR_NETWORK;
    }
    
    int parse_result = luarmor_parse_sync_info(response, info);
    free(response);
    
    if (parse_result != 0) {
        return LUARMOR_ERROR_JSON;
    }
    
    return LUARMOR_OK;
}

void luarmor_free_sync_info(luarmor_sync_info_t* info) {
    if (!info) return;
    
    if (info->nodes) {
        for (size_t i = 0; i < info->node_count; i++) {
            free(info->nodes[i]);
        }
        free(info->nodes);
        info->nodes = NULL;
    }
    info->node_count = 0;
}

luarmor_error_t luarmor_check_key(const luarmor_config_t* config, const char* key, const luarmor_sync_info_t* sync_info, luarmor_key_response_t* response) {
    if (!config || !key || !sync_info || !response) {
        return LUARMOR_ERROR_INVALID_RESPONSE;
    }
    
    if (sync_info->node_count == 0) {
        return LUARMOR_ERROR_INVALID_RESPONSE;
    }
    
    int random_index = rand() % sync_info->node_count;
    const char* selected_node = sync_info->nodes[random_index];
    
    char* client_nonce = luarmor_random_string(16);
    if (!client_nonce) {
        return LUARMOR_ERROR_MEMORY;
    }
    
    char server_time_str[32];
    snprintf(server_time_str, sizeof(server_time_str), "%d", sync_info->server_time);
    
    char signature_input[512];
    snprintf(signature_input, sizeof(signature_input), "%s%s%s%s%s%s%s",
             client_nonce, config->secret_n1,
             key, config->secret_n2,
             server_time_str, config->secret_n3,
             config->hwid);
    
    char* external_signature = luarmor_sha1_hash(signature_input);
    if (!external_signature) {
        free(client_nonce);
        return LUARMOR_ERROR_MEMORY;
    }
    
    char url[512];
    snprintf(url, sizeof(url), "%sexternal_check_key?by=%s&key=%s",
             selected_node, config->app_name, key);
    
    struct curl_slist* headers = NULL;
    char header_buf[256];
    
    snprintf(header_buf, sizeof(header_buf), "Content-Type: application/json");
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "clienttime: %d", sync_info->server_time);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "clientnonce: %s", client_nonce);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "clienthwid: %s", config->hwid);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "%s-fingerprint: %s", config->executor_name, config->hwid);
    headers = curl_slist_append(headers, header_buf);
    
    snprintf(header_buf, sizeof(header_buf), "externalsignature: %s", external_signature);
    headers = curl_slist_append(headers, header_buf);
    
    char* http_response = luarmor_http_get(url, headers);
    curl_slist_free_all(headers);
    free(external_signature);
    
    if (!http_response) {
        free(client_nonce);
        return LUARMOR_ERROR_NETWORK;
    }
    
    memset(response, 0, sizeof(luarmor_key_response_t));
    int parse_result = luarmor_parse_key_response(http_response, response);
    free(http_response);
    
    if (parse_result != 0) {
        free(client_nonce);
        return LUARMOR_ERROR_JSON;
    }
    
    if (response->code && strcmp(response->code, "KEY_VALID") == 0 && response->has_signature) {
        char verify_input[256];
        snprintf(verify_input, sizeof(verify_input), "%s%s%s",
                 client_nonce, config->secret_n3, response->code);
        
        char* expected_signature = luarmor_sha1_hash(verify_input);
        if (!expected_signature) {
            free(client_nonce);
            return LUARMOR_ERROR_MEMORY;
        }
        
        if (strcmp(expected_signature, response->signature) != 0) {
            free(expected_signature);
            free(client_nonce);
            return LUARMOR_ERROR_SIGNATURE_MISMATCH;
        }
        
        free(expected_signature);
    }
    
    free(client_nonce);
    return LUARMOR_OK;
}

void luarmor_free_key_response(luarmor_key_response_t* response) {
    if (!response) return;
    
    if (response->code) {
        free(response->code);
        response->code = NULL;
    }
    if (response->message) {
        free(response->message);
        response->message = NULL;
    }
    if (response->signature) {
        free(response->signature);
        response->signature = NULL;
    }
    if (response->note) {
        free(response->note);
        response->note = NULL;
    }
    response->has_signature = false;
}

bool luarmor_is_key_valid(const luarmor_key_response_t* response) {
    if (!response || !response->code) {
        return false;
    }
    return strcmp(response->code, "KEY_VALID") == 0;
}
