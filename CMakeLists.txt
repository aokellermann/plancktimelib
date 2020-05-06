cmake_minimum_required(VERSION 3.16)
project(planckclock C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

find_package(GMP REQUIRED)

option(BUILD_SHARED_LIBS "" True)

add_library("${PROJECT_NAME}" "")

add_subdirectory(include)
add_subdirectory(src)
add_subdirectory(test)

install(TARGETS "${PROJECT_NAME}" LIBRARY DESTINATION "lib" PUBLIC_HEADER DESTINATION "include")
