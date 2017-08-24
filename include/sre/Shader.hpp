/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once
#include "glm/glm.hpp"
#include "sre/Light.hpp"
#include "sre/BlendType.hpp"
#include "sre/WorldLights.hpp"
#include "sre/imgui_sre.hpp"
#include "sre/impl/Export.hpp"

#include "sre/impl/CPPShim.hpp"
#include <string>
#include <memory>
#include <vector>
#include <map>

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
        Texture,
        TextureCube,
        Invalid
    };

    const char* c_str(UniformType u);

    struct DllExport Uniform {
		std::string name;
        int id;
        UniformType type;
        int arraySize;                  // 1 means not array
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
     */
    class DllExport Shader : public std::enable_shared_from_this<Shader> {
    public:

        class DllExport ShaderBuilder {
        public:
            ShaderBuilder& withSource(const std::string& vertexShaderGLSL,
                                      const std::string& fragmentShaderGLSL);
            ShaderBuilder& withSourceStandard();
            ShaderBuilder& withSourceUnlit();
            ShaderBuilder& withSourceUnlitSprite();
            ShaderBuilder& withSourceStandardParticles();
            ShaderBuilder& withSourceDebugUV();
            ShaderBuilder& withSourceDebugNormals();
            ShaderBuilder& withOffset(float factor,float units);  // set the scale and units used to calculate depth values (note for WebGL1.0/OpenGL ES 2.0 only affects polygon fill)
            ShaderBuilder& withDepthTest(bool enable);
            ShaderBuilder& withDepthWrite(bool enable);
            ShaderBuilder& withBlend(BlendType blendType);
            ShaderBuilder& withName(const std::string& name);
            std::shared_ptr<Shader> build();
        private:
            ShaderBuilder() = default;
            ShaderBuilder(const ShaderBuilder&) = default;
            std::string vertexShaderStr;
            std::string fragmentShaderStr;
            bool depthTest = true;
            bool depthWrite = true;
            glm::vec2 offset = {0,0};
            std::string name;
            BlendType blend = BlendType::Disabled;
            friend class Shader;
        };

        static std::shared_ptr<Shader> getStandard();          // Phong Light Model. Uses light objects and ambient light set in Renderer.
                                                               // Uniforms
                                                               // "color" vec4 (default (1,1,1,1))
                                                               // "tex" shared_ptr<Texture> (default white texture)
                                                               // "specularity" float (default 0 = no specularity)
                                                               // VertexAttributes
                                                               // "position" vec3
                                                               // "normal" vec3
                                                               // "uv" vec4

        static std::shared_ptr<Shader> getUnlit();             // Unlit model.
                                                               // Uniforms
                                                               // "color" vec4 (default (1,1,1,1))
                                                               // "tex" shared_ptr<Texture> (default white texture)

        static std::shared_ptr<Shader> getUnlitSprite();       // UnlitSprite = no depth examples and alpha blending
                                                               // Uniforms
                                                               // "color" vec4 (default (1,1,1,1))
                                                               // "tex" shared_ptr<Texture> (default white texture)

        static std::shared_ptr<Shader> getStandardParticles(); // StandardParticles
                                                               // Uniforms
                                                               // "tex" shared_ptr<Texture> (default alpha sphere texture)

        static ShaderBuilder create();

        ~Shader();

        std::shared_ptr<Material> createMaterial();

        Uniform getUniformType(const std::string &name);

        std::pair<int,int> getAttibuteType(const std::string & name); // Return type, size of the attribute

        bool isDepthTest();

        bool isDepthWrite();

        BlendType getBlend();

        glm::vec2 getOffset();

        const std::string& getName();

        std::vector<std::string> getAttributeNames();
        std::vector<std::string> getUniformNames();

        // Validates the mesh attributes. If invalid, then set info variable to error message.
        // This method should be used for debug purpose only
        bool validateMesh(Mesh* mesh, std::string & info);

    private:
        bool setLights(WorldLights* worldLights, glm::mat4 viewTransform);

        Shader();

        bool build(const std::string& vertexShader, const std::string& fragmentShader);

        void bind();

        unsigned int shaderProgramId;
        bool depthTest = true;
        bool depthWrite = true;
        std::string name;
        BlendType blend = BlendType::Disabled;
        glm::vec2 offset = glm::vec2(0,0);

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

        int uniformLocationModel;
        int uniformLocationView;
        int uniformLocationProjection;
        int uniformLocationNormal;
        int uniformLocationViewport;
        int uniformLocationAmbientLight;
        int uniformLocationLightPosType;
        int uniformLocationLightColorRange;

    public:
        static std::string translateToGLSLES(std::string source, bool vertexShader);

    };
}