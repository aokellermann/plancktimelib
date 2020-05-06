set(FILES "${CMAKE_CURRENT_SOURCE_DIR}/planckclock.c")
set(RELATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

target_sources("${PROJECT_NAME}" PRIVATE ${FILES})

target_link_libraries("${PROJECT_NAME}" gmp)

#install(FILES ${FILES} DESTINATION "lib")
