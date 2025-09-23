cmake_minimum_required(VERSION 3.10)

project(simple-markdown CXX C)

add_subdirectory(vendor/webview)
add_subdirectory(vendor/cmark)

# if (RELEASE)
# 	set(CMAKE_WIN32_EXECUTABLE ON)
# endif()

# add_executable(${PROJECT_NAME} WIN32)
add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE src/main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)
target_link_libraries(${PROJECT_NAME} PRIVATE cmark)