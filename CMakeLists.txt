cmake_minimum_required(VERSION 3.10)
project(luarmor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CURL REQUIRED libcurl)
pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(CJSON REQUIRED libcjson)

add_library(luarmor STATIC
    src/luarmor.c
    src/utils/sha1.c
    src/utils/random.c
    src/utils/http.c
    src/utils/json_parser.c
)

target_include_directories(luarmor PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(luarmor PRIVATE
    ${CURL_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${CJSON_INCLUDE_DIRS}
)

target_link_libraries(luarmor PRIVATE
    ${CURL_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${CJSON_LIBRARIES}
)

target_compile_options(luarmor PRIVATE ${CURL_CFLAGS_OTHER} ${OPENSSL_CFLAGS_OTHER} ${CJSON_CFLAGS_OTHER})

add_executable(luarmor_test tests/test_luarmor.c)
target_link_libraries(luarmor_test PRIVATE luarmor ${CURL_LIBRARIES} ${OPENSSL_LIBRARIES} ${CJSON_LIBRARIES})
target_include_directories(luarmor_test PRIVATE ${CURL_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIRS} ${CJSON_INCLUDE_DIRS})

enable_testing()
add_test(NAME luarmor_tests COMMAND luarmor_test)
