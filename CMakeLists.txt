cmake_minimum_required(VERSION 3.2)
project(BudouXCppClient LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

find_package(nlohmann_json 3.10.5 REQUIRED)

add_subdirectory(simdutf)

set(SOURCE_FILES main.cpp)

add_executable(BudouXCppClient ${SOURCE_FILES})
target_link_libraries(BudouXCppClient PRIVATE simdutf nlohmann_json::nlohmann_json)
