cmake_minimum_required(VERSION 3.15)
project(esni_curl_api_proof C)

set(CMAKE_C_STANDARD 99)

add_executable(esni_curl_api_proof src/main.c)

include_directories(include)
include_directories(src)

#find_library(CGOLIB NAMES shapeshifter-meeklite-go.lib shapeshifter-meeklite-go HINTS lib)
#target_link_libraries(shapeshifter_plugin_meeklite ${CGOLIB} wsock32 ws2_32 Winmm)

#find_package(Threads REQUIRED)
find_library(CURLLIB NAMES libcurl.a libcurl HINTS lib)
find_library(SSLLIB NAMES libssl.a libssl HINTS lib)
find_library(CRYPTOLIB NAMES libcrypto.a libcrypto HINTS lib)
target_link_libraries(esni_curl_api_proof ${CURLLIB} ${SSLLIB} ${CRYPTOLIB} pthread dl)
