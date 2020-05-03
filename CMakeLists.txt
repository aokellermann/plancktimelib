cmake_minimum_required(VERSION 3.16)
project(planckclock C)

set(CMAKE_C_STANDARD 90)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

find_package(GMP REQUIRED)

add_executable(planckclock planckclock.c test.c)

target_link_libraries(planckclock gmp)

add_library(planckclocklib planckclock.c planckclock.h)
set_target_properties(planckclocklib PROPERTIES PUBLIC_HEADER "planckclock.h")
install(TARGETS planckclocklib LIBRARY DESTINATION "lib" PUBLIC_HEADER DESTINATION "include")
