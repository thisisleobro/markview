cmake_minimum_required(VERSION 3.10)

project(simple-markdown CXX)

add_subdirectory(vendor/webview)

# set subsystem windows to be able to use 'int WINAPI WinMain'
if (MSVC)
	set(CMAKE_WIN32_EXECUTABLE ON)
endif()

# add_executable(${PROJECT_NAME} WIN32)
add_executable(${PROJECT_NAME}
	src/main.cpp
)

# target_sources(${PROJECT_NAME} PRIVATE src/main.cpp src/utils.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)