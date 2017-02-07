# SimpleRenderEngine

The goal of SimpleRenderEngine (SRE) is to provide easy way to get started with graphics 
programming in 2D or 3D without a deep knowledge about the low-level graphics APIs like 
OpenGL, DirectX and Vulkan.
 
SRE currently depends on Simple Direct Layer 2.x (SDL2), SDL2-image, OpenGL Mathematics (GLM), and OpenGL 3.3 (or higher),
imgui and runs on both Windows, macOS and Linux.
 
SRE provides:
 * Virtual camera (perspective and orthographic)
 * Texture support (JPEG, PNG, Raw)
 * Mesh support (with normals and texture coordinates)
 * Mesh support (with color, particle-size and texture coordinates)
 * Shaders (unlit, specular, alpha blending, and custom shaders)
 * Light and shading
 * Enforces efficient use of OpenGL
 * Forward rendering
 * Full C++14 support
 * Support for 2D or 3D rendering
 * GUI rendering (using dear ImGui)
 * Emscripten support

To keep SRE as simple and flexible as possible the following features are not a part of SRE:
 * Resource management
 * Scenegraphs
 * Deferred rendering
 * Bump mapping
 * Shadowmap support
 * Dynamic particle systems

## Examples
 
Example usage can be found in the examples folder

## Documentation

Online documentation:
https://mortennobel.github.io/SimpleRenderEngine/
 
## Other resources
 
 * https://www.libsdl.org Simple Direct Layer 2.x 
 * https://www.libsdl.org/projects/SDL_image/ Simple Direct Layer Image 2.x
 * http://glm.g-truc.net/ OpenGL Mathematics
 * https://www.opengl.org/registry/ OpenGL Registry
 
## Version history
 
 * 0.4.1 Add RenderStats
 * 0.4.0 Merged Mesh and ParticleMesh into the Mesh class
 * 0.3.1 Creation of Light, Sphere(Mesh), Cube(Mesh) and Quad(Mesh) using Builder pattern.
 * 0.3.0 Creation of Texture, Mesh and Shader using Builder pattern.
 * 0.2.9 Fixed issues with specular highlight
 * 0.2.8 Added Emscripten support
 * 0.2.7 Add more mesh topologies (line strip, triangle strip and triangle fans). Add Debug::drawLineStrip(). 
 * 0.2.6 Fix point-light attenuation 
 * 0.2.5 Set Camera default viewport
 * 0.2.4 Restructured include folder and CMake build. Fix particle bug.
 * 0.2.2 ParticleMesh Support for OpenGL 3.1
 * 0.2.1 Make particle size depend on viewport. Move internal headers into impl.  
 * 0.2 Move specularity from Light to a normal shader uniform (since logically it belongs to material instead of the light source)  
   - Add ParticleMesh 
   - Add support for imgui 
   - Remove simple text rendering
   - Refactoring Shader: verify uniform names and types.
 * 0.1 Initial version
 
 ## Build verification
 * Coverty <a href="https://scan.coverity.com/projects/mortennobel-simplerenderengine">
   <img alt="Coverity Scan Build Status"
        src="https://scan.coverity.com/projects/11679/badge.svg"/>
 </a>