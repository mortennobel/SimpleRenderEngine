# Renderer

The goal of SimpleRenderEngine (sre) is to provide easy way to get started with graphics 
programming in 2D or 3D without a deep knowledge about the low-level graphics APIs like 
OpenGL, DirectX and Vulkan.
 
SimpleRenderEngine currently depends on Simple Direct Layer 2.x (SDL2), SDL2-image, OpenGL Mathematics (GLM), and OpenGL 
3.1 (or higher), imgui and runs on both Windows, macOS and Linux.
 
sre provides:
 * Virtual camera (perspective and orthographic)
 * Texture support (JPEG, PNG, Raw)
 * Cube map support
 * Mesh support (with custom vertex attributes)
 * Shaders (unlit, specular, alpha blending, and custom shaders)
 * Light and shading
 * Enforces efficient use of OpenGL
 * Forward rendering
 * Full C++14 support
 * Support for 2D or 3D rendering
 * GUI rendering (using dear ImGui)
 * Emscripten support (allows cross compiling to HTML 5 + WebGL)

To keep sre as simple and flexible as possible the following features are not a part of sre:
 * Scenegraphs
 * Deferred rendering
 * Bump mapping
 * Shadowmap support
 * Dynamic particle systems

## Examples
 
Example usage can be found in the examples folder

## Documentation

API documentation is defined in header files.
 
## Other resources
 
 * https://www.libsdl.org Simple Direct Layer 2.x 
 * https://www.libsdl.org/projects/SDL_image/ Simple Direct Layer Image 2.x
 * http://glm.g-truc.net/ OpenGL Mathematics
 * https://www.opengl.org/registry/ OpenGL Registry
 * https://github.com/ocornut/imgui ImGui 1.51
