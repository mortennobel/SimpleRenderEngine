file(GLOB SOURCE_FILES
	"sre/*.cpp"
	"../include/sre/*.hpp"
	"../include/sre/impl/*.hpp"
    "imgui/*.cpp"
)

include_directories(.)

add_library(SRE STATIC ${SOURCE_FILES})
target_link_libraries(SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

install(TARGETS SRE DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)
