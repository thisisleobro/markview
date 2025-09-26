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

if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE src/WinMain.c)
endif()

include_directories(${PROJECT_NAME} include)

# cmark stuff
# add_subdirectory(vendor/cmark)
add_subdirectory(vendor/cmark-gfm)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/extensions)
include_directories(${PROJECT_NAME} build/vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} build/vendor/cmark-gfm/extensions)
# target_link_libraries(${PROJECT_NAME} PRIVATE cmark)
target_link_libraries(${PROJECT_NAME} PRIVATE libcmark-gfm libcmark-gfm-extensions)

# webview stuff
add_subdirectory(vendor/webview)
target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)