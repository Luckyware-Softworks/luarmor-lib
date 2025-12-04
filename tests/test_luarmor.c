#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/luarmor.h"

void test_random_string() {
    char* str1 = luarmor_random_string(16);
    assert(str1 != NULL);
    assert(strlen(str1) == 16);
    
    char* str2 = luarmor_random_string(16);
    assert(str2 != NULL);
    assert(strlen(str2) == 16);
    assert(strcmp(str1, str2) != 0);
    
    free(str1);
    free(str2);
    printf("test_random_string: PASSED\n");
}

void test_sha1_hash() {
    const char* input = "test";
    char* hash = luarmor_sha1_hash(input);
    assert(hash != NULL);
    assert(strlen(hash) == 40);
    
    const char* expected = "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3";
    assert(strcmp(hash, expected) == 0);
    
    free(hash);
    printf("test_sha1_hash: PASSED\n");
}

void test_sha1_empty() {
    char* hash = luarmor_sha1_hash("");
    assert(hash != NULL);
    const char* expected = "da39a3ee5e6b4b0d3255bfef95601890afd80709";
    assert(strcmp(hash, expected) == 0);
    free(hash);
    printf("test_sha1_empty: PASSED\n");
}

void test_signature_calculation() {
    const char* client_nonce = "s2mle100lesh420f";
    const char* secret_n1 = "asdfdg";
    const char* key = "BAfjuLxndwTvMBNiCyqMsXMaTcOqXpcr";
    const char* secret_n2 = "zxczxcv";
    const char* server_time = "1739703913";
    const char* secret_n3 = "hjgh";
    const char* hwid = "03b3b409-f0b97340-40b97304-48327b49827";
    
    char signature_input[512];
    snprintf(signature_input, sizeof(signature_input), "%s%s%s%s%s%s%s",
             client_nonce, secret_n1, key, secret_n2, server_time, secret_n3, hwid);
    
    char* hash = luarmor_sha1_hash(signature_input);
    assert(hash != NULL);
    assert(strlen(hash) == 40);
    free(hash);
    printf("test_signature_calculation: PASSED\n");
}

void test_sync_info_fetch() {
    luarmor_sync_info_t info = {0};
    luarmor_error_t result = luarmor_fetch_sync_info(&info);
    
    if (result == LUARMOR_OK) {
        assert(info.server_time > 0);
        assert(info.node_count > 0);
        assert(info.nodes != NULL);
        
        for (size_t i = 0; i < info.node_count; i++) {
            assert(info.nodes[i] != NULL);
            assert(strlen(info.nodes[i]) > 0);
        }
        
        luarmor_free_sync_info(&info);
        printf("test_sync_info_fetch: PASSED\n");
    } else {
        printf("test_sync_info_fetch: SKIPPED (network error: %d)\n", result);
    }
}

void test_key_check_invalid() {
    luarmor_sync_info_t sync_info = {0};
    luarmor_error_t sync_result = luarmor_fetch_sync_info(&sync_info);
    
    if (sync_result != LUARMOR_OK) {
        printf("test_key_check_invalid: SKIPPED (sync failed)\n");
        return;
    }
    
    luarmor_config_t config = {
        .secret_n1 = "test_secret_1",
        .secret_n2 = "test_secret_2",
        .secret_n3 = "test_secret_3",
        .app_name = "testapp",
        .hwid = "test-hwid-12345",
        .executor_name = "testexec"
    };
    
    luarmor_key_response_t response = {0};
    luarmor_error_t result = luarmor_check_key(&config, "INVALID_KEY_12345678901234567890", &sync_info, &response);
    
    if (result == LUARMOR_OK) {
        assert(response.code != NULL);
        assert(strcmp(response.code, "KEY_VALID") != 0);
        luarmor_free_key_response(&response);
        printf("test_key_check_invalid: PASSED\n");
    } else if (result == LUARMOR_ERROR_NETWORK) {
        printf("test_key_check_invalid: SKIPPED (network error)\n");
    } else {
        printf("test_key_check_invalid: PASSED (error code: %d)\n", result);
    }
    
    luarmor_free_sync_info(&sync_info);
}

void test_is_key_valid() {
    luarmor_key_response_t response1 = {0};
    response1.code = strdup("KEY_VALID");
    assert(luarmor_is_key_valid(&response1) == true);
    free(response1.code);
    
    luarmor_key_response_t response2 = {0};
    response2.code = strdup("KEY_INVALID");
    assert(luarmor_is_key_valid(&response2) == false);
    free(response2.code);
    
    luarmor_key_response_t response3 = {0};
    assert(luarmor_is_key_valid(&response3) == false);
    
    assert(luarmor_is_key_valid(NULL) == false);
    printf("test_is_key_valid: PASSED\n");
}

void test_memory_cleanup() {
    luarmor_sync_info_t info = {0};
    luarmor_fetch_sync_info(&info);
    luarmor_free_sync_info(&info);
    assert(info.nodes == NULL);
    assert(info.node_count == 0);
    
    luarmor_key_response_t response = {0};
    response.code = strdup("TEST");
    response.message = strdup("TEST");
    response.signature = strdup("TEST");
    response.note = strdup("TEST");
    luarmor_free_key_response(&response);
    assert(response.code == NULL);
    assert(response.message == NULL);
    assert(response.signature == NULL);
    assert(response.note == NULL);
    printf("test_memory_cleanup: PASSED\n");
}

int main() {
    printf("Running Luarmor library tests...\n\n");
    
    test_random_string();
    test_sha1_hash();
    test_sha1_empty();
    test_signature_calculation();
    test_is_key_valid();
    test_memory_cleanup();
    test_sync_info_fetch();
    test_key_check_invalid();
    
    printf("\nAll tests completed!\n");
    return 0;
}

