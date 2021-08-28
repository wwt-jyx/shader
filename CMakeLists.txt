cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 11)

project("shader")
SET(PROJECT_NAME shader)
SET(LIBRARY_NAME Shared)
SET(PROJECT_BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/build)
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#########################################################
# FIND OPENGL
#########################################################
find_package(OpenGL REQUIRED)
include_directories(${OpenGL_INCLUDE_DIRS})
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
if(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
endif(NOT OPENGL_FOUND)

#########################################################
# FIND assimp
#########################################################
set(CMAKE_MODULE_PATH "/home/nsy/Downloads/assimp-4.1.0/cmake-modules/;${CMAKE_MODULE_PATH}")
find_package(assimp REQUIRED)
include_directories(${assimp_INCLUDE_DIRS})
link_directories(${assimp_LIBRARY_DIRS})
add_definitions(${assimp_DEFINITIONS})
#########################################################
# glad
#########################################################
add_library(glad SHARED ./src/glad.c)
#########################################################
# stb_image
#########################################################
add_library(stb_image SHARED ./src/stb_image.cpp)

## Our include directories.
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

## Generate our own shared library.
FILE(GLOB_RECURSE SRC_FILES src/*.cpp)
FILE(GLOB_RECURSE HEAD_FILES include/*.h)

ADD_LIBRARY(Shared SHARED ${SRC_FILES})

## Specify executable source file.
ADD_EXECUTABLE(${PROJECT_NAME} src/main.cpp)

## Add all link directories to project.
TARGET_LINK_LIBRARIES(${PROJECT_NAME} Shared glad "/usr/local/lib/libassimp.so.4" glfw3 X11 stb_image ${OPENGL_LIBRARIES}  -ldl -lpthread -lm)
