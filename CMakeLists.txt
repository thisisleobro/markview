cmake_minimum_required(VERSION 3.10)

project(markview CXX C)

# add_executable(${PROJECT_NAME} WIN32)
add_executable(${PROJECT_NAME})

# if (RELEASE)
# 	set(CMAKE_WIN32_EXECUTABLE ON)
# endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        src/main.c
        src/markdown.c
        src/file.c
)

set(CMARK_SHARED OFF)
set(CMARK_STATIC ON)
set(CMARK_GFM_STATIC_DEFINE 1)
set(CMARK_TESTS OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/WinMain.c)
endif()

include_directories(${PROJECT_NAME} include)

# embed stuff
include(cmake/FileEmbed.cmake)
FileEmbedSetup()
FileEmbedAdd(README.md)
target_link_libraries(${PROJECT_NAME} PRIVATE file_embed)

# cmark stuff
add_subdirectory(vendor/cmark-gfm)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/extensions)
include_directories(${PROJECT_NAME} build/vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} build/vendor/cmark-gfm/extensions)
# target_link_libraries(${PROJECT_NAME} PRIVATE cmark)
target_link_libraries(${PROJECT_NAME} PRIVATE libcmark-gfm-extensions_static libcmark-gfm_static)

# webview stuff
add_subdirectory(vendor/webview)
target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)

# SDL stuff
add_subdirectory(vendor/SDL)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
