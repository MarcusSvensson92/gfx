# Minimum required CMake version
cmake_minimum_required(VERSION 3.8.2 FATAL_ERROR)

# Project
project(Samples)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/01_HelloTriangle")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/02_TexturedQuad")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/03_Compute")