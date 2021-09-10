cmake_minimum_required(VERSION 3.20)
project(spigotclipbuilder)

set(CMAKE_CXX_STANDARD 17)

add_executable(spigotclipbuilder src/main.cpp)

include(FetchContent)

FetchContent_Declare(json
        GIT_REPOSITORY https://github.com/nlohmann/json
        GIT_TAG v3.10.2)

FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

target_link_libraries(spigotclipbuilder PRIVATE nlohmann_json::nlohmann_json)