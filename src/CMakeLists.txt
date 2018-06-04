file(GLOB SOURCE_FILES
	"${CMAKE_CURRENT_SOURCE_DIR}/sre/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/sre/impl/*.cpp"
	"../include/sre/*.hpp"
	"../include/sre/impl/*.hpp"
)

add_library(SRE STATIC ${SOURCE_FILES} ${EXTRA_SOURCE_FILES})
target_link_libraries(SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

install(TARGETS SRE DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)
