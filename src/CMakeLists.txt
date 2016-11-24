file(GLOB SOURCE_FILES
	"SRE/*.cpp"
	"../include/SRE/*.hpp"
	"../include/SRE/impl/*.hpp"
    "imgui/*.cpp"
)

add_library(SRE STATIC ${SOURCE_FILES})
target_link_libraries(SRE ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES})

install(TARGETS SRE DESTINATION lib)
install(DIRECTORY ../include/ DESTINATION include)
