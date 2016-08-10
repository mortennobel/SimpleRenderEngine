cmake_minimum_required(VERSION 3.5)
project(SimpleRenderEngine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(SOURCE_FILES SRE/Texture.cpp SRE/Camera.cpp SRE/Light.cpp SRE/Mesh.cpp SRE/SimpleRenderEngine.cpp SRE/Shader.cpp SRE/Text.cpp SRE/Text.hpp)

add_executable(SRE-Spinning-Cube ${SOURCE_FILES} test/spinning-cube.cpp)
add_executable(SRE-Spinning-Sphere ${SOURCE_FILES} test/spinning-sphere.cpp)
add_executable(SRE-Spinning-Cube-Unlit ${SOURCE_FILES} test/spinning-cube-unlit.cpp)
add_executable(SRE-Spinning-Sphere-Unlit ${SOURCE_FILES} test/spinning-sphere-unlit.cpp)
add_executable(SRE-Spinning-Cube-Tex ${SOURCE_FILES} test/spinning-cube-tex.cpp)

add_library(SRE SHARED ${SOURCE_FILES})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

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
	include_directories(${OpenGL_INCLUDE_DIRS})
	find_package(glew REQUIRED)
	include_directories(${GLEW_INCLUDE_DIR})
	SET(EXTRA_LIBS ${OPENGL_LIBRARY} ${GLEW_LIBRARY})
ENDIF (APPLE)

find_package(SDL2_IMAGE REQUIRED)
include_directories(${SDL2_IMAGE_INCLUDE_DIRS})
include_directories(.)

# GLM -- on Windows: depends on env var: GLM_ROOT_DIR
find_package (glm REQUIRED)
IF( GLM_FOUND )
    INCLUDE_DIRECTORIES( ${GLM_INCLUDE_DIR} )
ELSE( GLM_FOUND )
    MESSAGE( STATUS "Failed to find GLM ----------------------")
ENDIF( GLM_FOUND )

target_link_libraries(SRE-Spinning-Cube ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(SRE-Spinning-Sphere ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(SRE-Spinning-Cube-Unlit ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(SRE-Spinning-Sphere-Unlit ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(SRE-Spinning-Cube-Tex ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})
target_link_libraries(SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

IF (WIN32)
file(COPY test-data DESTINATION Debug)
file(COPY test-data DESTINATION Release)
SET(SDL2_dll "" CACHE STRING "Where the SDL2.dll can be found")
SET(SDL2_image_dll "" CACHE STRING "Where the SDL2_image.dll can be found")
SET(libjpeg-9_dll "" CACHE STRING "Where the libjpeg-9.dll can be found")
SET(libpng16-16_dll "" CACHE STRING "Where the libpng16-16.dll can be found")
SET(libtiff-5_dll "" CACHE STRING "Where the libtiff-5.dll can be found")
SET(libwebp-4_dll "" CACHE STRING "Where the libwebp-4.dll can be found")
SET(zlib1_dll "" CACHE STRING "Where the zlib1.dll can be found")
SET(DLLFileList ${libjpeg-9_dll} ${libpng16-16_dll} ${SDL2_dll} ${SDL2_image_dll} ${zlib1_dll} ${libtiff-5_dll} ${libwebp-4_dll})
file(COPY ${DLLFileList} DESTINATION Debug)
file(COPY ${DLLFileList} DESTINATION Release)
ENDIF(WIN32)