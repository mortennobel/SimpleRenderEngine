cmake_minimum_required(VERSION 3.5)
project(SimpleRenderEngine CXX)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})
include_directories(sre)
include_directories(include)

#########################################################
# FIND OPENGL
#########################################################
IF(APPLE)
    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (OpenGL_LIBRARY)
    SET(EXTRA_LIBS ${OpenGL_LIBRARY})
ELSE (APPLE)
    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})
    IF(UNIX)
        find_package(GLEW REQUIRED)
    ELSE(UNIX)
        find_package(glew REQUIRED)
    ENDIF(UNIX)
    include_directories(${GLEW_INCLUDE_DIR})
    SET(EXTRA_LIBS ${OPENGL_LIBRARY} ${GLEW_LIBRARY})
ENDIF (APPLE)

find_package(SDL2_IMAGE REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})

IF (WIN32)
    SET(SDL2_dll "" CACHE STRING "Where the SDL2.dll can be found")
    SET(SDL2_image_lib_path "" CACHE PATH "Where the SDL2_image.dll can be found")
    SET(DLLFileList "${SDL2_image_lib_path}/SDL2_image.dll" "${SDL2_image_lib_path}/libjpeg-9.dll" "${SDL2_image_lib_path}/libpng16-16.dll" "${SDL2_image_lib_path}/libtiff-5.dll" "${SDL2_image_lib_path}/libwebp-4.dll" "${SDL2_image_lib_path}/zlib1.dll"  ${SDL2_dll})
ENDIF(WIN32)

add_subdirectory(src)
add_subdirectory(examples)
add_subdirectory(test)
