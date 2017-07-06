//
// Created by morten on 31/07/16.
//

#pragma once
#include "glm/glm.hpp"
#include "sre/Light.hpp"
#include "sre/BlendType.hpp"
#include "sre/WorldLights.hpp"

#include "sre/impl/Export.hpp"

#include "sre/impl/CPPShim.hpp"
#include <string>
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

    struct DllExport Uniform {
		std::string name;
        int id;
        UniformType type;
        // 1 means not array
        int arraySize;
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
     *      - tex (Texture*) (default white texture)
     *      - specular (float) (default 0.0) (means no specular)
     * - Shader::getUnlit()
     *    - Uses the camera states as well as the color and texture parameters to define the surface color
     *    - Parameters:
     *      - color (vec4) (default white)
     *      - tex (Texture*) (default white texture)
     * - Shader::getUnlitSprite()
     *    - Similar to getUnlit() but with no depth write
     *    - Parameters:
     *      - color (vec4) (default white)
     *      - tex (Texture*) (default white texture)
     * - Shader::getStandardParticles()
     *    - Similar to getUnlitSprite() but with no depth write
     *    - Parameters:
     *      - tex (Texture*) (default alpha sphere texture)
     *
     *   Shaders have two kinds of uniforms variables:
     *     - Global uniforms (prefixed with 'g_' which is automatically set by the engine)
     *     - Material uniform (without 'g_' prefix). Which are exposed to materials.
     */
    class DllExport Shader {
    public:

        class DllExport ShaderBuilder {
        public:
            ShaderBuilder& withSource(const std::string& vertexShaderGLSL, const std::string& fragmentShaderGLSL);
            ShaderBuilder& withSourceStandard();
            ShaderBuilder& withSourceUnlit();
            ShaderBuilder& withSourceUnlitSprite();
            ShaderBuilder& withSourceStandardParticles();
            ShaderBuilder& withSourceDebugUV();
            ShaderBuilder& withSourceDebugNormals();
            ShaderBuilder& withDepthTest(bool enable);
            ShaderBuilder& withDepthWrite(bool enable);
            ShaderBuilder& withBlend(BlendType blendType);
            std::shared_ptr<Shader> build();
        private:
            ShaderBuilder() = default;
            ShaderBuilder(const ShaderBuilder&) = default;
            std::string vertexShaderStr;
            std::string fragmentShaderStr;
            bool depthTest = true;
            bool depthWrite = true;
            BlendType blend = BlendType::Disabled;
            friend class Shader;
        };

        // Phong Light Model. Uses light objects and ambient light set in Renderer.
        // Attributes
        // "color" vec4 (default (1,1,1,1))
        // "tex" Texture* (default white texture)
        // "specularity" float (default 0 = no specularity)
        static std::shared_ptr<Shader> getStandard();
        // Unlit model.
        // Attributes
        // "color" vec4 (default (1,1,1,1))
        // "tex" Texture* (default white texture)
        static std::shared_ptr<Shader> getUnlit();
        // UnlitSprite = no depth examples and alpha blending
        // Attributes
        // "color" vec4 (default (1,1,1,1))
        // "tex" Texture* (default white texture)
        static std::shared_ptr<Shader> getUnlitSprite();
        // StandardParticles
        // Attributes
        // "tex" Texture* (default alpha sphere texture)
        static std::shared_ptr<Shader> getStandardParticles();

        static ShaderBuilder create();

        ~Shader();

        Uniform getUniformType(const std::string &name);

        // return element type, element count
        std::pair<int,int> getAttributeType(const std::string &name);

        bool isDepthTest();

        bool isDepthWrite();

        BlendType getBlend();

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
        BlendType blend = BlendType::Disabled;

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