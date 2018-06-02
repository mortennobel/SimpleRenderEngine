set(EXTRA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/imgui ${CMAKE_CURRENT_SOURCE_DIR}/glm ${CMAKE_CURRENT_SOURCE_DIR}/picojson  CACHE PATH "" FORCE)

file(GLOB EXTRA_SOURCE_FILES_
    "imgui/imgui.cpp"
    "imgui/imgui_draw.cpp"
)

if (NOT EXTRA_SOURCE_FILES_)
    message("Cannot find modules. Run 'git submodule update --init' or checkout SRE using 'git clone --recurse-submodules https://github.com/mortennobel/SimpleRenderEngine.git'")
endif()

set(EXTRA_SOURCE_FILES ${EXTRA_SOURCE_FILES_} CACHE PATH "" FORCE)