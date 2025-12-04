#include <cjson/cJSON.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../../include/luarmor.h"

int luarmor_parse_sync_info(const char* json_str, luarmor_sync_info_t* info) {
    cJSON* json = cJSON_Parse(json_str);
    if (!json) return -1;
    
    cJSON* st = cJSON_GetObjectItem(json, "st");
    cJSON* nodes = cJSON_GetObjectItem(json, "nodes");
    
    if (!st || !nodes || !cJSON_IsArray(nodes)) {
        cJSON_Delete(json);
        return -1;
    }
    
    info->server_time = cJSON_GetNumberValue(st);
    
    size_t count = cJSON_GetArraySize(nodes);
    info->nodes = (char**)malloc(count * sizeof(char*));
    if (!info->nodes) {
        cJSON_Delete(json);
        return -1;
    }
    
    info->node_count = 0;
    for (size_t i = 0; i < count; i++) {
        cJSON* node = cJSON_GetArrayItem(nodes, i);
        if (cJSON_IsString(node)) {
            const char* node_str = cJSON_GetStringValue(node);
            info->nodes[info->node_count] = strdup(node_str);
            if (!info->nodes[info->node_count]) {
                for (size_t j = 0; j < info->node_count; j++) {
                    free(info->nodes[j]);
                }
                free(info->nodes);
                cJSON_Delete(json);
                return -1;
            }
            info->node_count++;
        }
    }
    
    cJSON_Delete(json);
    return 0;
}

int luarmor_parse_key_response(const char* json_str, luarmor_key_response_t* response) {
    cJSON* json = cJSON_Parse(json_str);
    if (!json) return -1;
    
    cJSON* code = cJSON_GetObjectItem(json, "code");
    cJSON* message = cJSON_GetObjectItem(json, "message");
    cJSON* signature = cJSON_GetObjectItem(json, "signature");
    cJSON* data = cJSON_GetObjectItem(json, "data");
    
    if (code && cJSON_IsString(code)) {
        response->code = strdup(cJSON_GetStringValue(code));
    }
    if (message && cJSON_IsString(message)) {
        response->message = strdup(cJSON_GetStringValue(message));
    }
    if (signature && cJSON_IsString(signature)) {
        response->signature = strdup(cJSON_GetStringValue(signature));
        response->has_signature = true;
    } else {
        response->has_signature = false;
    }
    
    if (data && cJSON_IsObject(data)) {
        cJSON* auth_expire = cJSON_GetObjectItem(data, "auth_expire");
        cJSON* total_executions = cJSON_GetObjectItem(data, "total_executions");
        cJSON* note = cJSON_GetObjectItem(data, "note");
        
        if (auth_expire && cJSON_IsNumber(auth_expire)) {
            response->auth_expire = (int32_t)cJSON_GetNumberValue(auth_expire);
        }
        if (total_executions && cJSON_IsNumber(total_executions)) {
            response->total_executions = (int32_t)cJSON_GetNumberValue(total_executions);
        }
        if (note && cJSON_IsString(note)) {
            response->note = strdup(cJSON_GetStringValue(note));
        }
    }
    
    cJSON_Delete(json);
    return 0;
}
