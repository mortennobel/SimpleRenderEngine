#!/bin/bash

EMSDK=/Users/mnob/programming/cpp/emsdk_portable
source ${EMSDK}/emsdk_env.sh

for FILENAME in SingleFileOpenGLTex hello-engine gui spheres particle-sprite particle-test spinning-cube-tex multiple-lights spinning-sphere-cubemap spinning-cube benchmark64k
do
echo $FILENAME
emcc -Iinclude src/imgui/imgui.cpp \
               src/imgui/imgui_draw.cpp \
               src/imgui/imgui_sre.cpp \
               src/sre/Camera.cpp \
               src/sre/Light.cpp \
               src/sre/Material.cpp \
               src/sre/Mesh.cpp \
               src/sre/Renderer.cpp \
               src/sre/RenderPass.cpp \
               src/sre/SDLRenderer.cpp \
               src/sre/Shader.cpp \
               src/sre/Texture.cpp \
               src/sre/WorldLights.cpp \
               examples/$FILENAME.cpp \
               -O2 -std=c++14 -s TOTAL_MEMORY=33554432 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --preload-file examples/data -s USE_SDL=2 -o html/$FILENAME.html
done