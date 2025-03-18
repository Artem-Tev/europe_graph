cmake_minimum_required(VERSION 3.29)
project(europe_graph)

set(CMAKE_CXX_STANDARD 14)
# nlohmann/json
include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# cpr
FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 1.9.3
)
set(CPR_USE_SYSTEM_CURL OFF)
FetchContent_MakeAvailable(cpr)

add_executable(europe_graph
    graph.cpp)
target_link_libraries(europe_graph PRIVATE nlohmann_json::nlohmann_json cpr::cpr)