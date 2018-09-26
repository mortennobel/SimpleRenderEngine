cmake_minimum_required(VERSION 3.5)
project(SimpleRenderEngine CXX)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(WARNING
            "Building from within the SimpleRenderEngine source tree is not recommended.\n"
            "Hint: Run these commands\n"
            "$ rm -rf CMakeCache.txt CMakeFiles\n"
            "$ mkdir -p ../sre_build\n"
            "$ cd ../sre_build\n"
            "$ cmake ${CMAKE_CURRENT_SOURCE_DIR}")
endif ()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
set(SRE_INCLUDE_DIRS ${SDL2_INCLUDE_DIR})

#########################################################
# FIND OPENGL
#########################################################
IF(APPLE)
    add_definitions(-DGL_SILENCE_DEPRECATION)
    list(APPEND SRE_INCLUDE_DIRS /System/Library/Frameworks)
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (OpenGL_LIBRARY)
    SET(EXTRA_LIBS ${OpenGL_LIBRARY})
ELSE (APPLE)
    find_package(OpenGL REQUIRED)
    list(APPEND SRE_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})
    IF(UNIX)
        find_package(GLEW REQUIRED)
    ELSE(UNIX)
        find_package(glew REQUIRED)
    ENDIF(UNIX)
    list(APPEND SRE_INCLUDE_DIRS ${GLEW_INCLUDE_DIR})
    SET(EXTRA_LIBS ${OPENGL_LIBRARY} ${GLEW_LIBRARY})
ENDIF (APPLE)

find_package(SDL2_IMAGE REQUIRED)
list(APPEND SRE_INCLUDE_DIRS ${SDL2_IMAGE_INCLUDE_DIRS})

option(USE_OPENVR "Enable OpenVR" OFF)

set(OPENVR_LIB)
IF (USE_OPENVR)
	find_package(OpenVR REQUIRED)
    list(APPEND SRE_INCLUDE_DIRS ${OPENVR_INCLUDE_DIRS})
	IF(UNIX)
		add_definitions(-DSRE_OPENVR)
	ELSE(UNIX)	
		add_definitions(/DSRE_OPENVR)
	ENDIF(UNIX)
    IF(APPLE)
        if(${OPENVR_PLATFORM} MATCHES "osx32")
            set(OPENVR_LIB "${OPENVR_ROOT_DIR}/lib/${OPENVR_PLATFORM}/libopenvr_api.dylib")
        else()
            set(OPENVR_LIB "${OPENVR_ROOT_DIR}/lib/${OPENVR_PLATFORM}/OpenVR.framework")
        endif()
        MESSAGE("${OPENVR_LIB}")
    ELSE(APPLE)
        set(OPENVR_LIB "${OPENVR_ROOT_DIR}/lib/${OPENVR_PLATFORM}/openvr_api.lib")
    ENDIF(APPLE)
ENDIF(USE_OPENVR)

add_subdirectory(submodules)
set(SRE_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include ${EXTRA_INCLUDE} ${SRE_INCLUDE_DIRS} CACHE PATH "" FORCE)
include_directories(${SRE_INCLUDE})

add_subdirectory(src)
SET(USE_SRE_EXAMPLES ON CACHE BOOL "Enable examples for SRE")
if (USE_SRE_EXAMPLES)
    add_subdirectory(examples)
endif(USE_SRE_EXAMPLES)

SET(USE_SRE_TEST_AND_UTILS OFF CACHE BOOL "Enable Tests and Utils for SRE")
IF (USE_SRE_TEST_AND_UTILS)
    add_subdirectory(test)
    add_subdirectory(utils)
ENDIF(USE_SRE_TEST_AND_UTILS)
set(SRE_LIBRARIES SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} ${OPENVR_LIB} CACHE PATH "" FORCE)