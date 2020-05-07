add_executable(testpc test.c)
target_include_directories(testpc PRIVATE "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(testpc PRIVATE "${PROJECT_NAME}")