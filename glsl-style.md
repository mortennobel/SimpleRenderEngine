## GLSL programming style

Simple Render Engine provides some coding styles:

* Varying (in/out) is prefixed with 'v'
* World space positions/directions is prefixed with 'ws'
* Eye space (relative to camera) is prefixed with 'es'
* Built-in uniforms is prefixed with 'g_'

### Built in uniforms

**Transforms and viewport**

* glm::mat4 **g_model** Model transform, which transforms from local space to world space. 
* glm::mat4 **g_view** View transform, which transforms from world space to eye space.
* glm::mat4 **g_projection** Projection transform, which transforms from eye space to clip space.
* glm::mat3 **g_model_it** Model transform inverse transpose, used to transforms normals local space to world space.
* glm::mat3 **g_model_view_it** Model-View inverse transpose, used to transforms normals local space to eye space.
* glm::mat3 **g_viewport** viewportSize (xy) and viewportOffset(zw)

**Light**

* glm::vec4 **g_ambientLight**. Automatically set from WorldLight::ambientLight (w is ignored)
* glm::vec4[] **g_lightPosType**. Contains a number of scene light positions (.xyz) and types (.w). (w==0 is directional light, w==1 is point light)
* glm::vec4[] **g_lightColorRange**. Contains a number of scene light colors (.xyz) and range (.w). 

### Vertex attributes

Simple render engine uses dynamic vertex attribute, where mapping between a vertex shader input and a mesh is determined runtime.
The built in shaders uses the following attributes:

* vec3 **position**
* vec3 **normal**
* vec4 **uv**
* vec4 **tangent**
* vec4 **color**

### Shader specializations

Shaders can be specialized using specialization constants. These constants are injected as shader preprocessor 
symbols (e.g. "#define S_PI 3.14"). Shader specialization constants must start with "S_".

To instantiate a specialized shader, use:

```
auto mat = shader->createMaterial({{"S_PI","3.14"}});
```

This will instantiate a new shader and keep a reference in the main shader shader, which makes sure that the specialized 
shader is only instantiated once.

There are also defined a number of engine specific definitions, which cannot be changed. All prefixed with "SI_":

* **SI_LIGHTS** number of lights per draw call. The number is defined as a engine constant.
* **SI_FRAMEBUFFER_SRGB** defined when framebuffers is sRGB.
* **SI_TEX_SAMPLER_SRGB** defined when texture sampler supports sRGB.
* **SI_VERTEX** Defined only for vertex shaders.
* **SI_FRAGMENT** Defined only for fragment shaders.
* **SI_GEOMETRY** Defined only for geometry shaders.
* **SI_TESS_CTRL** Defined only for tesselation control shaders.
* **SI_TESS_EVAL** Defined only for tesselation evaluation shaders.


### Overwriting built-in shaders

