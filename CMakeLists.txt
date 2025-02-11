cmake_minimum_required(VERSION 3.15)

project(hashmap VERSION 0.0.1 LANGUAGES C)

option(HASHMAP_BUILD_TEST "Build Test" ON)
option(HASHMAP_LIB "Build Lib" OFF)

if(HASHMAP_BUILD_TEST)
    add_subdirectory(test)
endif()

if(HASHMAP_LIB)
    add_library(hashmap hashmap.c MurmurHash3.c)
    target_include_directories(hashmap PRIVATE include)
endif()