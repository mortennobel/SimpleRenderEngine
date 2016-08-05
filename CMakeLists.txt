cmake_minimum_required(VERSION 3.5)
project(SimpleRenderEngine)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(SOURCE_FILES SRE/Texture.cc SRE/Camera.cc SRE/Light.cc SRE/Mesh.cc SRE/SimpleRenderEngine.cc SRE/Shader.cc)

add_executable(SRE-Spinning-Cube ${SOURCE_FILES} test/spinning-cube.cpp)

add_executable(SRE-Spinning-Sphere ${SOURCE_FILES} test/spinning-sphere.cpp)

add_executable(SRE-Spinning-Cube-Tex ${SOURCE_FILES} test/spinning-cube-tex.cpp)

find_package(SDL2 REQUIRED)
#########################################################
# FIND OPENGL
#########################################################
include_directories(${SDL2_INCLUDE_DIR})
IF(APPLE)
    INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
    FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
    MARK_AS_ADVANCED (OpenGL_LIBRARY)
    SET(EXTRA_LIBS ${OpenGL_LIBRARY})
ELSE(TRUE)
    find_package(OpenGL REQUIRED)
    include_directories(${OpenGL_INCLUDE_DIRS})
    SET(EXTRA_LIBS ${OPENGL_LIBRARY})
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
target_link_libraries(SRE-Spinning-Cube-Tex ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})
