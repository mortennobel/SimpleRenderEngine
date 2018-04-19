/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once
#include "glm/glm.hpp"
#include "sre/Light.hpp"
#include "sre/BlendType.hpp"
#include "sre/WorldLights.hpp"
#include "sre/imgui_sre.hpp"
#include "sre/impl/Export.hpp"
#include "sre/impl/GL.hpp"
#include "sre/impl/CPPShim.hpp"

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <set>


namespace sre {

    class Mesh;
    class Texture;
    class Material;

    enum class UniformType {
        Int,
        Float,
        Mat3,
        Mat4,
        Vec3,
        Vec4,
        IVec4,
        Texture,
        TextureCube,
        Invalid
    };

    enum class ShaderType {
        Vertex,
        Fragment,
        Geometry,
        TessellationControl,
        TessellationEvaluation,
        NumberOfShaderTypes
    };

    uint32_t to_id(ShaderType st);

    const char* c_str(UniformType u);

    struct DllExport Uniform {
		std::string name;
        int id;
        UniformType type;
        int arraySize;                  // 1 means not array
    };

    enum class StencilFunc {
        Never = GL_NEVER,               // Never pass.
        Less = GL_LESS,                 // Pass if (ref & mask) <  (stencil & mask).
        Equal = GL_EQUAL,               // Pass if (ref & mask) =  (stencil & mask).
        LEqual = GL_LEQUAL,             // Pass if (ref & mask) <= (stencil & mask).
        Greater = GL_GREATER,           // Pass if (ref & mask) >  (stencil & mask).
        NotEqual = GL_NOTEQUAL,         // Pass if (ref & mask) != (stencil & mask).
        GEequal = GL_GEQUAL,            // Pass if (ref & mask) >= (stencil & mask).
        Always = GL_ALWAYS,             // Always pass.
        Disabled                        // Stencil test disabled
    };

    enum class StencilOp {
        Keep = GL_KEEP,                 // Keeps the current value.
        Zero = GL_ZERO,                 // Sets the stencil buffer value to 0.
        Replace = GL_REPLACE,           // Sets the stencil buffer value to the reference value as specified by WebGLRenderingContext.stencilFunc().
        Incr = GL_INCR,                 // Increments the current stencil buffer value. Clamps to the maximum representable unsigned value.
        IncrWrap = GL_INCR_WRAP,        // Increments the current stencil buffer value. Wraps stencil buffer value to zero when incrementing the maximum representable unsigned value.
        Decr = GL_DECR,                 // Decrements the current stencil buffer value. Clamps to 0.
        DecrWrap = GL_DECR_WRAP,        // Decrements the current stencil buffer value. Wraps stencil buffer value to the maximum representable unsigned value when decrementing a stencil buffer value of 0.
        Invert = GL_INVERT,             // Inverts the current stencil buffer value bitwise.
    };

    struct Stencil {
        StencilFunc func = StencilFunc::Disabled; // Specifying the test function
        uint16_t ref;                             // Specifying the reference value for the stencil test. This value is clamped to the range 0 to 2n -1 where n is the number of bitplanes in the stencil buffer.
        uint16_t mask;                            // Specifying a bit-wise mask that is used to AND the reference value and the stored stencil value when the test is done.
        StencilOp fail = StencilOp::Keep;         // Specifying the function to use when the stencil test fails.
        StencilOp zfail = StencilOp::Keep;        // Specifying the function to use when the stencil test passes, but the depth test fails
        StencilOp zpass = StencilOp::Keep;        // Specifying the function to use when both the stencil test and the depth test pass, or when the stencil test passes and there is no depth buffer or depth testing is disabled
    };

    /**
     * Controls the appearance of the rendered objects.
     *
     * The shader also controls depth test, depth write and blending.
     *
     * There is the following premade shaders:
     * - Shader::getStandard()
     *    - Shades the mesh using the Phong light model, and uses the current light states and camera states as well as
     *      the color and texture parameters
     *    - Parameters:
     *      - color (vec4) (default white)
     *      - tex (shared_ptr<Texture>) (default white texture)
     *      - specular (float) (default 0.0) (means no specular)
     * - Shader::getStandardPhong()
     *    - Shades the mesh using the Phong light model, and uses the current light states and camera states as well as
     *      the color and texture parameters
     *    - Parameters:
     *      - color (vec4) (default white)
     *      - tex (shared_ptr<Texture>) (default white texture)
     *      - specular (float) (default 0.0) (means no specular)
     * - Shader::getUnlit()
     *    - Uses the camera states as well as the color and texture parameters to define the surface color
     *    - Parameters:
     *      - color (vec4) (default white)
     *      - tex (shared_ptr<Texture>) (default white texture)
     * - Shader::getUnlitSprite()
     *    - Similar to getUnlit() but with no depth write
     *    - color (vec4) (vertex attribute)
     *    - Parameters:
     *      - tex (shared_ptr<Texture>) (default white texture)
     * - Shader::getStandardParticles()
     *    - Similar to getUnlitSprite() but with no depth write
     *    - Parameters:
     *      - tex (shared_ptr<Texture>) (default alpha sphere texture)
     *
     *   Shaders have two kinds of uniforms variables:
     *     - Global uniforms (prefixed with 'g_' which is automatically set by the engine)
     *     - Material uniform (without 'g_' prefix). Which are exposed to materials.
     *
     *   Shaders can be specialized using specialization constants, which is a list of key-value pairs, used to define
     *   special behavior of shaders. Specialization constants (key-values) are translated to preprocessor symbols in
     *   the shader code.
     *   Using specialized shaders is useful for performance reasons - only enabling features that the shader needs. But
     *   creating a specialized shader (as well as creating shaders in general) may caurse performance issues and should
     *   avoid during realtime rendering.
     *   Specialization constants must start start with 'S_' and must consist of capital letters, digits and underscore.
     */
    class DllExport Shader : public std::enable_shared_from_this<Shader> {
        enum class ResourceType{
            File,
            Memory
        };

        struct Resource{
            ResourceType resourceType;
            std::string value;
        };
    public:

        class DllExport ShaderBuilder {
        public:
            DEPRECATED("Use ShaderType withSourceString() or withSourceFile() instead")
            ShaderBuilder& withSource(const std::string& vertexShaderGLSL,
                                      const std::string& fragmentShaderGLSL);
            ShaderBuilder& withSourceString(const std::string& shaderSource, ShaderType shaderType);
            ShaderBuilder& withSourceFile(const std::string& shaderFile, ShaderType shaderType);
            ShaderBuilder& withOffset(float factor,float units);  // set the scale and units used to calculate depth values (note for WebGL1.0/OpenGL ES 2.0 only affects polygon fill)
            ShaderBuilder& withDepthTest(bool enable);
            ShaderBuilder& withDepthWrite(bool enable);
            ShaderBuilder& withColorWrite(glm::bvec4 enable);
            ShaderBuilder& withBlend(BlendType blendType);
            ShaderBuilder& withStencil(Stencil stencil);
            ShaderBuilder& withName(const std::string& name);
            std::shared_ptr<Shader> build(std::vector<std::string>& errors);
            std::shared_ptr<Shader> build();
            ShaderBuilder(const ShaderBuilder&) = default;
        private:
            ShaderBuilder(Shader* shader);
            ShaderBuilder() = default;
            std::map<ShaderType, Resource> shaderSources;
            std::map<std::string,std::string> specializationConstants;
            bool depthTest = true;
            bool depthWrite = true;
            glm::bvec4 colorWrite = glm::bvec4(true, true, true, true);
            glm::vec2 offset = {0,0};
            std::string name;
            Shader *updateShader = nullptr;
            BlendType blend = BlendType::Disabled;
            Stencil stencil = {};
            friend class Shader;
        };

        DEPRECATED("Use getStandardPBR or getStandardBlinnPhong")
        static std::shared_ptr<Shader> getStandard();

        static std::shared_ptr<Shader> getStandardPBR();       // Phong Light Model. Uses light objects and ambient light set in Renderer.
                                                               // Uniforms
                                                               //   "color" vec4 (default (1,1,1,1))
                                                               //   "tex" shared_ptr<Texture> (default white texture)
                                                               //   "metallicRoughness" vec4 (default 0,0) x = metallic, y = roughness
                                                               // VertexAttributes
                                                               //   "position" vec3
                                                               //   "normal" vec3
                                                               //   "uv" vec4
                                                               // Specializations
                                                               // S_METALROUGHNESSMAP
                                                               //   Adds Uniforms "mrTex" (Texture). rg-channel used for metallic and roughness
                                                               // S_TANGENTS
                                                               //   Adds VertexAttribute "tangent" vec4. Used for normal maps. Otherwise compute using
                                                               // S_NORMALMAP
                                                               //   Adds Uniforms "normalTex" (Texture) and "normalScale" (float)
                                                               // S_EMISSIVEMAP
                                                               //   Adds Uniforms "emissiveTex" (Texture) and "emissiveFactor" (vec4)
                                                               // S_OCCLUSIONMAP
                                                               //   Adds Uniforms "occlusionTex" (Texture) and "occlusionStrength" (float)
                                                               // S_VERTEX_COLOR
                                                               //   Adds VertexAttribute "color" vec4 defined in linear space.


        static std::shared_ptr<Shader> getStandardBlinnPhong(); // Blinn-Phong Light Model. Uses light objects and ambient light set in Renderer.
                                                                // Uniforms
                                                                //   "color" vec4 (default (1,1,1,1))
                                                                //   "tex" shared_ptr<Texture> (default white texture)
                                                                // "specularity" float (default 0 = no specularity)
                                                                // VertexAttributes
                                                                //   "position" vec3
                                                                //   "normal" vec3
                                                                //   "uv" vec4
                                                                // Specializations
                                                                // S_VERTEX_COLOR
                                                                //   Adds VertexAttribute "color" vec4 defined in linear space.

        static std::shared_ptr<Shader> getStandardPhong();      // Similar to Blinn-Phong, but with more accurate specular highlights

        static std::shared_ptr<Shader> getUnlit();             // Unlit model.
                                                               // Uniforms
                                                               //   "color" vec4 (default (1,1,1,1))
                                                               //   "tex" shared_ptr<Texture> (default white texture)
                                                               // Specializations
                                                               // S_VERTEX_COLOR
                                                               //   Adds VertexAttribute "color" vec4 defined in linear space.

        static std::shared_ptr<Shader> getSkybox();

        static std::shared_ptr<Shader> getSkyboxProcedural();

        static std::shared_ptr<Shader> getUnlitSprite();       // UnlitSprite = no depth examples and alpha blending
                                                               // Uniforms
                                                               //   "color" vec4 (default (1,1,1,1))
                                                               //   "tex" shared_ptr<Texture> (default white texture)

        static std::shared_ptr<Shader> getStandardParticles(); // StandardParticles
                                                               // Uniforms
                                                               //   "tex" shared_ptr<Texture> (default alpha sphere texture)
                                                               // VertexAttributes
                                                               //   "position" vec3
                                                               //   "particleSize" float
                                                               //   "uv" vec4 (note: xy is lower left corner, z is size and w is rotation in radians)
                                                               // Expects a mesh with topology = Points

        static std::shared_ptr<Shader> getBlit();             // Shader used for blitting
                                                              // Uniforms
                                                              //   "tex" shared_ptr<Texture> (default white texture)

        static ShaderBuilder create();
        ShaderBuilder update();                                // Update the shader using the builder pattern. (Must end with build()).

        ~Shader();

        std::shared_ptr<Material> createMaterial(std::map<std::string,std::string> specializationConstants = {});

        Uniform getUniform(const std::string &name);

        std::pair<int,int> getAttibuteType(const std::string & name); // Return type, size of the attribute

        bool isDepthTest();

        bool isDepthWrite();

        glm::bvec4 getColorWrite();

        BlendType getBlend();

        glm::vec2 getOffset();

        Stencil getStencil();

        const std::string& getName();

        std::vector<std::string> getAttributeNames();
        std::vector<std::string> getUniformNames();

        // Validates the mesh attributes. If invalid, then set info variable to error message.
        // This method should be used for debug purpose only
        bool validateMesh(Mesh* mesh, std::string & info);

        std::map<std::string,std::string> getCurrentSpecializationConstants();

        std::set<std::string> getAllSpecializationConstants();
    private:
        std::string precompile(std::string source, std::vector<std::string>& errors, uint32_t shaderType);
        std::string insertPreprocessorDefines(std::string source,
                                              std::map<std::string, std::string> &specializationConstants,
                                              uint32_t shaderType);

        bool setLights(WorldLights* worldLights);

        Shader();

        std::map<std::string,std::string> specializationConstants = {};

        std::shared_ptr<Shader> parent = nullptr;
        std::vector<std::weak_ptr<Shader>> specializations;

        bool build(std::map<ShaderType,Resource> shaderSources, std::vector<std::string>& errors);
        static std::string getSource(Resource& resource);
        bool compileShader(Resource& resource, GLenum type, GLuint& shader, std::vector<std::string>& errors);
        void bind();

        unsigned int shaderProgramId = 0;
        bool depthTest = true;
        bool depthWrite = true;
        long shaderUniqueId = 0;
        glm::bvec4 colorWrite = glm::bvec4(true, true, true, true);
        std::string name;
        BlendType blend = BlendType::Disabled;
        glm::vec2 offset = glm::vec2(0,0);
        Stencil stencil;

        std::map<ShaderType, Resource> shaderSources;

        std::vector<Uniform> uniforms;

        struct ShaderAttribute {
            int32_t position;
            unsigned int type;
            int32_t arraySize;
        };

        std::map<std::string, ShaderAttribute> attributes;
        void updateUniformsAndAttributes();

        friend class Mesh;
        friend class Material;
        friend class RenderPass;
        friend class Inspector;

        int uniformLocationModel;
        int uniformLocationView;
        int uniformLocationProjection;
        int uniformLocationModelInverseTranspose;
        int uniformLocationModelViewInverseTranspose;
        int uniformLocationViewport;
        int uniformLocationAmbientLight;
        int uniformLocationLightPosType;
        int uniformLocationLightColorRange;
        int uniformLocationCameraPosition;

    public:
        static std::string translateToGLSLES(std::string source, bool vertexShader, int version = 100);
    };
}