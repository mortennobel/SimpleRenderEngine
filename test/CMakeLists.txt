# List of single-file tests
SET(scr_files spinning-sphere-cubemap particle-test polygon-offset-example multiple-lights particle-sprite sprite-test multi-cameras static_vertex_attribute custom-mesh-layout-default-values imgui_demo texture-test screen-point-to-ray)

# Create custom build targets
FOREACH(scr_file ${scr_files})
    add_executable(SRE-Test-${scr_file} ${scr_file}.cpp)
    target_link_libraries(SRE-Test-${scr_file} ${EXTRA_LIBS} ${SDL2_LIBRARY} ${SDL2_IMAGE_LIBRARIES} SRE)
ENDFOREACH()

file(COPY ../test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
file(COPY ../test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)

IF (WIN32)
    SET(SDL2_dll "" CACHE STRING "Where the SDL2.dll can be found")
    SET(SDL2_image_lib_path "" CACHE PATH "Where the SDL2_image.dll can be found")
    SET(DLLFileList "${SDL2_image_lib_path}/SDL2_image.dll" "${SDL2_image_lib_path}/libjpeg-9.dll" "${SDL2_image_lib_path}/libpng16-16.dll" "${SDL2_image_lib_path}/libtiff-5.dll" "${SDL2_image_lib_path}/libwebp-4.dll" "${SDL2_image_lib_path}/zlib1.dll"  ${SDL2_dll})
    file(COPY ${DLLFileList} DESTINATION Debug)
    file(COPY ${DLLFileList} DESTINATION Release)
ENDIF(WIN32)
