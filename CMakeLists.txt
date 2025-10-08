cmake_minimum_required(VERSION 3.10)

project(markview CXX C)

# add_executable(${PROJECT_NAME} WIN32)
add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release,ON>)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})

if(WIN32)
	message("Building ${PROJECT_NAME} for windows")
    add_compile_definitions(MARKVIEW_WINDOWS)
elseif(APPLE)
	message("Building ${PROJECT_NAME} for apple")
    add_compile_definitions(MARKVIEW_APPLE)
elseif(UNIX AND NOT APPLE)
	message("Building ${PROJECT_NAME} for linux")
    add_compile_definitions(MARKVIEW_LINUX)
else()
	message(FATAL_ERROR "unknown OS")
endif()

set(BUILD_SHARED_LIBS OFF)
set(CMARK_SHARED OFF)
set(CMARK_STATIC ON)
set(CMARK_TESTS OFF)
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_AUDIO OFF)
# SDL_VIDEO is needed to create a window
set(SDL_VIDEO ON)
set(SDL_GPU OFF)
set(SDL_RENDER ON)
set(SDL_CAMERA OFF)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_HIDAPI OFF)
set(SDL_POWER OFF)
set(SDL_SENSOR OFF)
set(SDL_DIALOG OFF)

add_compile_definitions(-DCMARK_GFM_STATIC_DEFINE=1)
add_compile_definitions(-DCMARK_GFM_EXTENSIONS_STATIC_DEFINE=1)

include_directories(${PROJECT_NAME} include)
add_compile_definitions(-DCMAKE_EXE_LINKER_FLAGS="-static")
target_link_libraries(${PROJECT_NAME} PRIVATE -static-libstdc++)

# embed stuff
include(cmake/FileEmbed.cmake)
FileEmbedSetup()
FileEmbedAdd(assets/welcome.md)
FileEmbedAdd(assets/prism.min.js)
FileEmbedAdd(assets/prism.css)
target_link_libraries(${PROJECT_NAME} PRIVATE file_embed)

# cmark stuff
add_subdirectory(vendor/cmark-gfm)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} vendor/cmark-gfm/extensions)
include_directories(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/vendor/cmark-gfm/src)
include_directories(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/vendor/cmark-gfm/extensions)
# target_link_libraries(${PROJECT_NAME} PRIVATE cmark)
target_link_libraries(${PROJECT_NAME} PRIVATE libcmark-gfm-extensions_static libcmark-gfm_static)

# webview stuff
add_subdirectory(vendor/webview)
target_link_libraries(${PROJECT_NAME} PRIVATE webview::core_static)

# SDL stuff
add_subdirectory(vendor/SDL)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)

# cJSON stuff
add_subdirectory(vendor/cJSON)
target_link_libraries(${PROJECT_NAME} PRIVATE cjson)
include_directories(${PROJECT_NAME} vendor/cJSON)
