set(EXTRA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/ImGuiColorTextEdit ${CMAKE_CURRENT_SOURCE_DIR}/imgui ${CMAKE_CURRENT_SOURCE_DIR}/glm ${CMAKE_CURRENT_SOURCE_DIR}/picojson  CACHE PATH "" FORCE)

file(GLOB EXTRA_SOURCE_FILES_
    "ImGuiColorTextEdit/TextEditor.cpp"
    "imgui/imgui.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_widgets.cpp"
)

if (NOT EXTRA_SOURCE_FILES_)
    message( FATAL_ERROR "Cannot find submodules used in SimpleRenderEngine. Run 'git submodule update --init' or checkout full project using 'git clone --recurse-submodules <url>'")
endif()

set(EXTRA_SOURCE_FILES ${EXTRA_SOURCE_FILES_} CACHE PATH "" FORCE)