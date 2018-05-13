[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/mortennobel/SimpleRenderEngine/master/LICENSE)
[![Build Status](https://travis-ci.org/mortennobel/SimpleRenderEngine.svg?branch=master)](https://travis-ci.org/mortennobel/SimpleRenderEngine)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/86403818b8b54161a6fef03248c0b828)](https://www.codacy.com/app/mortennobel/SimpleRenderEngine?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=mortennobel/SimpleRenderEngine&amp;utm_campaign=Badge_Grade)

# Renderer

The goal of SimpleRenderEngine (sre) is to provide easy way to get started with graphics 
programming in 2D or 3D without a deep knowledge about the low-level graphics APIs like 
OpenGL, DirectX and Vulkan.
 
SimpleRenderEngine currently depends on Simple Direct Layer 2.x (SDL2), SDL2-image, OpenGL Mathematics (GLM), and OpenGL 
3.3 (or higher), Dear ImGui and runs on both Windows, macOS and Linux.
 
sre provides:
 * Virtual camera (perspective and orthographic)
 * Texture support (JPEG, PNG)
 * Cube map support
 * Mesh support (with custom vertex attributes)
 * Shaders (PBR, Blinn-Phong, unlit, alpha blending, and custom shaders)
 * Enforces efficient use of OpenGL
 * Forward rendering
 * Full C++14 support
 * Support for 2D or 3D rendering
 * GUI rendering (using Dear ImGui)
 * Emscripten support (allows cross compiling to HTML 5 + WebGL)
 * VR support (OpenVR)
 * Bump mapping
 * Shadowmap

To keep sre as simple and flexible as possible the following features are not a part of sre:
 * Scenegraphs
 * Deferred rendering
 * Dynamic particle systems

## Examples
 
Example usage can be found in the examples folder.

[![Matcap](https://mortennobel.github.io/SimpleRenderEngine/examples/07_matcap.png)](https://mortennobel.github.io/SimpleRenderEngine/examples/07_matcap.html)[![Picking](https://mortennobel.github.io/SimpleRenderEngine/examples/09_picking.png)](https://mortennobel.github.io/SimpleRenderEngine/examples/09_picking.html)[![Skybox](https://mortennobel.github.io/SimpleRenderEngine/examples/10_skybox-example.png)](https://mortennobel.github.io/SimpleRenderEngine/examples/10_skybox-example.html)[![Render to texture](https://mortennobel.github.io/SimpleRenderEngine/examples/12_render-to-texture.png)](https://mortennobel.github.io/SimpleRenderEngine/examples/12_render-to-texture.html)[![Cloth_Simulation](https://mortennobel.github.io/SimpleRenderEngine/examples/15_cloth_simulation.png)](https://mortennobel.github.io/SimpleRenderEngine/examples/15_cloth_simulation.html)[![Shadows](https://mortennobel.github.io/SimpleRenderEngine/examples/16_shadows.png)](https://mortennobel.github.io/SimpleRenderEngine/examples/16_shadows.html)


More examples/projects can be found on https://github.com/mortennobel/SimpleRenderEngineProject which also contains
all/most dependencies for Windows and macOS.

## Documentation

API documentation is defined in header files.
 
## Other resources
 
 * https://www.libsdl.org Simple Direct Layer 2.x 
 * https://www.libsdl.org/projects/SDL_image/ Simple Direct Layer Image 2.x
 * http://glm.g-truc.net/ OpenGL Mathematics (bundled)
 * https://www.opengl.org/registry/ OpenGL Registry
 * https://github.com/ocornut/imgui ImGui 1.60 (bundled)
 * https://github.com/BalazsJako/ImGuiColorTextEdit ImGuiColorTextEdit (bundled)
 * https://github.com/kazuho/picojson PicoJSON(bundled)
 
