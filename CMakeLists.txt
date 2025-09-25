cmake_minimum_required(VERSION 3.10)

project(simple-markdown CXX C)

add_subdirectory(vendor/webview)
# add_subdirectory(vendor/cmark)
add_subdirectory(vendor/cmark-gfm)

# if (RELEASE)
# 	set(CMAKE_WIN32_EXECUTABLE ON)
# endif()


add_executable(${PROJECT_NAME})
# add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME} PRIVATE src/main.c)

include_directories(${PROJECT_NAME} vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/extensions)
include_directories(${PROJECT_NAME} build/vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} build/vendor/cmark-gfm/extensions)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/WinMain.c)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)
# target_link_libraries(${PROJECT_NAME} PRIVATE cmark)
target_link_libraries(${PROJECT_NAME} PRIVATE libcmark-gfm libcmark-gfm-extensions)
