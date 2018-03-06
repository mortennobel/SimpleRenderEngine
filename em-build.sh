#!/bin/bash

if [ -z "$EMSDK" ]
then
      echo "\$EMSDK is not defined. Using ~/programming/cpp/emsdk_portable"
      EMSDK=~/programming/cpp/emsdk_portable
fi

source ${EMSDK}/emsdk_env.sh

for FILENAME in multiple-materials multiple-lights imgui-color-test pbr-test custom-mesh-layout-default-values imgui_demo multi-cameras particle-sprite particle-test polygon-offset-example spinning-sphere-cubemap sprite-test static_vertex_attribute texture-test
do
echo $FILENAME
emcc -Iinclude src/imgui/imgui.cpp \
               src/imgui/imgui_draw.cpp \
               src/imgui/imgui_sre.cpp \
               src/imgui/TextEditor.cpp \
               src/sre/Camera.cpp \
               src/sre/Color.cpp \
               src/sre/Light.cpp \
               src/sre/Material.cpp \
               src/sre/Mesh.cpp \
               src/sre/Renderer.cpp \
               src/sre/RenderPass.cpp \
               src/sre/SDLRenderer.cpp \
               src/sre/Shader.cpp \
               src/sre/Texture.cpp \
               src/sre/WorldLights.cpp \
               src/sre/Framebuffer.cpp \
               src/sre/ModelImporter.cpp \
               src/sre/Sprite.cpp \
               src/sre/SpriteBatch.cpp \
               src/sre/SpriteAtlas.cpp \
               src/sre/Inspector.cpp \
               src/sre/Log.cpp \
               test/$FILENAME.cpp \
               -O2 -std=c++14 -s USE_WEBGL2=1 -s FORCE_FILESYSTEM=1 -s TOTAL_MEMORY=67108864 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file test_data -s USE_SDL=2 -o html/$FILENAME.html
done

for FILENAME in benchmark64k custom-mesh-layout gui hello-engine hello-engine-raw obj-viewer picking render-to-texture spheres spinning-cube spinning-primitives-tex sprite-example
do
echo $FILENAME
emcc -Iinclude src/imgui/imgui.cpp \
               src/imgui/imgui_draw.cpp \
               src/imgui/imgui_sre.cpp \
               src/imgui/TextEditor.cpp \
               src/sre/Camera.cpp \
               src/sre/Color.cpp \
               src/sre/Light.cpp \
               src/sre/Material.cpp \
               src/sre/Mesh.cpp \
               src/sre/Renderer.cpp \
               src/sre/RenderPass.cpp \
               src/sre/SDLRenderer.cpp \
               src/sre/Shader.cpp \
               src/sre/Texture.cpp \
               src/sre/WorldLights.cpp \
               src/sre/Framebuffer.cpp \
               src/sre/ModelImporter.cpp \
               src/sre/Sprite.cpp \
               src/sre/SpriteBatch.cpp \
               src/sre/SpriteAtlas.cpp \
               src/sre/Inspector.cpp \
               src/sre/Log.cpp \
               examples/$FILENAME.cpp \
               -O2 -std=c++14 -s USE_WEBGL2=1 -s FORCE_FILESYSTEM=1 -s TOTAL_MEMORY=33554432 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file examples_data -s USE_SDL=2 -o html/$FILENAME.html
done
