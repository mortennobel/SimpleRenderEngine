file(GLOB SOURCE_FILES
	"SRE/*.cpp"
	"../include/SRE/*.hpp"
	"../include/SRE/impl/*.hpp"
    "imgui/*.cpp"
)

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
include_directories(.)

add_library(SRE STATIC ${SOURCE_FILES})
target_link_libraries(SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

install(TARGETS SRE DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)
