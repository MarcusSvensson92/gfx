# Minimum required CMake version
cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)

# Project
project(Gfx)

# Source files
file(GLOB_RECURSE GFX_SOURCE_FILES
     "${CMAKE_CURRENT_SOURCE_DIR}/Include/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
	 "${CMAKE_CURRENT_SOURCE_DIR}/External/*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/External/*.cpp")

# Organize source files in folder groups
get_filename_component(ABSOLUTE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE)
foreach(SOURCE_FILE ${GFX_SOURCE_FILES})
	file(RELATIVE_PATH GROUP ${ABSOLUTE_PATH} ${SOURCE_FILE})
	string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" GROUP ${GROUP})
	string(REPLACE / \\ GROUP ${GROUP})
    source_group("${GROUP}" FILES ${SOURCE_FILE})
endforeach()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()
    
# Set include directory
include_directories("$ENV{VULKAN_SDK}/Include/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/External/Include/")

# Create library
add_library(Gfx STATIC ${GFX_SOURCE_FILES})

# Library directory
if(${CMAKE_GENERATOR} STREQUAL "Visual Studio 15 2017 Win64")
    set(GFX_VULKAN_LIB vulkan-1)
else()
    message(FATAL_ERROR "Unsupported generator!")
endif()

# Link libraries
target_link_libraries(Gfx ${GFX_VULKAN_LIB})
