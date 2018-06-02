file(GLOB SOURCE_FILES
	"sre/*.cpp"
	"sre/impl/*.cpp"
	"../include/sre/*.hpp"
	"../include/sre/impl/*.hpp"
)

include_directories(.)
message(${EXTRA_SOURCE_FILES})
add_library(SRE STATIC ${SOURCE_FILES} ${EXTRA_SOURCE_FILES})
target_link_libraries(SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

install(TARGETS SRE DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)
