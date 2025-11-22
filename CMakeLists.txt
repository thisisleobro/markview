cmake_minimum_required(VERSION 3.10)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW) # so that call to option does not discard already set values

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

project(markview CXX C)

# add_executable(${PROJECT_NAME} WIN32)
add_executable(${PROJECT_NAME})

set_target_properties(${PROJECT_NAME} PROPERTIES
	WIN32_EXECUTABLE $<$<CONFIG:Release>:ON>
)

# Debug, Release, RelWithDebInfo and MinSizeRel
message("${PROJECT_NAME}: building in \"${CMAKE_BUILD_TYPE}\" mode")


file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})

if(WIN32)
	message("${PROJECT_NAME}: Building for windows")
    add_compile_definitions(MARKVIEW_WINDOWS)
elseif(APPLE)
	message("${PROJECT_NAME}: Building for apple")
    add_compile_definitions(MARKVIEW_APPLE)
elseif(UNIX AND NOT APPLE)
	message("${PROJECT_NAME}: Building for linux")
    add_compile_definitions(MARKVIEW_LINUX)
else()
	message(FATAL_ERROR "unknown OS")
endif()

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

cmake_path(GET CMAKE_C_COMPILER PARENT_PATH BINFOLDER)
cmake_path(GET BINFOLDER PARENT_PATH TOOLCHAINFOLDER)

find_library(
	LIBUNWIND_LIBUNWIND
	NAMES
		unwind
		libunwind
		lunwind
	PATHS
	    ${TOOLCHAINFOLDER}/aarch64-w64-mingw32/lib
)

if (LIBUNWIND_LIBUNWIND)
    message(STATUS "Found libunwind: ${LIBUNWIND_LIBUNWIND}")
	target_link_libraries(${PROJECT_NAME} PRIVATE -lunwind -static)
endif()

# embed stuff
include(cmake/FileEmbed.cmake)
FileEmbedSetup()
FileEmbedAdd(assets/welcome.md)
FileEmbedAdd(assets/prism.min.js)
FileEmbedAdd(assets/prism.css)
FileEmbedAdd(assets/markview.js)
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
set(CJSON_OVERRIDE_BUILD_SHARED_LIBS ON)
set(CJSON_BUILD_SHARED_LIBS OFF)
add_subdirectory(vendor/cJSON)
target_link_libraries(${PROJECT_NAME} PRIVATE cjson)
include_directories(${PROJECT_NAME} vendor/cJSON)
