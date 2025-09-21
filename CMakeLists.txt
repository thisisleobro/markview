cmake_minimum_required(VERSION 3.10)

project(simple-markdown CXX)

add_subdirectory(vendor/webview)

add_executable(${PROJECT_NAME}
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)