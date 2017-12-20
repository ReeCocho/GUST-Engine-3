# Minimum CMake Version
cmake_minimum_required(VERSION 3.7)

# Project
project(GUST)

# Modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# Packages
find_package(SDL2 REQUIRED)
find_package(VULKAN REQUIRED)
find_package(GLM REQUIRED)
find_package(BULLET REQUIRED)

# Includes
include_directories(${CMAKE_SOURCE_DIR}/dependencies)
include_directories(${SDL2_INCLUDE_DIR})
include_directories(${VULKAN_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIRS})
include_directories(${Bullet_INCLUDE_DIRS})

# Release build
set(CMAKE_BUILD_TYPE Release)

# Add src subdirectory
add_subdirectory(src)