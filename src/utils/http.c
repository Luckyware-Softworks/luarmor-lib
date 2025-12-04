#include <curl/curl.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct {
    char* data;
    size_t size;
} http_response_t;

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    http_response_t* mem = (http_response_t*)userp;
    
    char* ptr = realloc(mem->data, mem->size + realsize + 1);
    if (!ptr) return 0;
    
    mem->data = ptr;
    memcpy(&(mem->data[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->data[mem->size] = 0;
    
    return realsize;
}

char* luarmor_http_get(const char* url, struct curl_slist* headers) {
    CURL* curl = curl_easy_init();
    if (!curl) return NULL;
    
    http_response_t response = {0};
    response.data = malloc(1);
    if (!response.data) {
        curl_easy_cleanup(curl);
        return NULL;
    }
    response.size = 0;
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        free(response.data);
        curl_easy_cleanup(curl);
        return NULL;
    }
    
    curl_easy_cleanup(curl);
    return response.data;
}
