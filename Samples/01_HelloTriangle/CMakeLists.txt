# Minimum required CMake version
cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)

# Project
project(01_HelloTriangle)

# Source files
file(GLOB_RECURSE SOURCE_FILES
     "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
     "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/../External/*.c"
     "${CMAKE_CURRENT_SOURCE_DIR}/../External/*.cpp")
     
# Organize source files in folder groups
get_filename_component(ABSOLUTE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/" ABSOLUTE)
foreach(SOURCE_FILE ${SOURCE_FILES})
	file(RELATIVE_PATH GROUP ${ABSOLUTE_PATH} ${SOURCE_FILE})
	string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" GROUP ${GROUP})
	string(REPLACE / \\ GROUP ${GROUP})
    source_group("${GROUP}" FILES ${SOURCE_FILE})
endforeach()

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

if(NOT TARGET Gfx)
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../.." Gfx)
endif()

# Create output directory
execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_SOURCE_DIR}/Bin")

# Set output directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/Bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/Bin")

if(${CMAKE_GENERATOR} STREQUAL "Visual Studio 15 2017 Win64")
    # Declare libraries
    set(GFX_LIBRARY_DIR "vs2017_x64")
    set(GFX_LIBRARIES Gfx glfw3)
    
    # Copy glslangValidator to output directory
    execute_process(COMMAND ${CMAKE_COMMAND} -E copy
        "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe"
        "${CMAKE_CURRENT_SOURCE_DIR}/Bin")
else()
    message(FATAL_ERROR "Unsupported generator!")
endif()

# Set include directory
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../Include/"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../External/Include/")
                    
# Set library directory
link_directories("$ENV{VULKAN_SDK}/Lib/"
                 "${CMAKE_CURRENT_SOURCE_DIR}/../External/Lib/${GFX_LIBRARY_DIR}/")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME} ${GFX_LIBRARIES})

# Set working directory for Visual Studio
set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Bin")