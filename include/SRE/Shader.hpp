//
// Created by morten on 31/07/16.
//

#pragma once
#include "glm/glm.hpp"
#include "SRE/Light.hpp"
#include "SRE/BlendType.hpp"

#include "SRE/impl/Export.hpp"

#include "SRE/impl/CPPShim.hpp"
#include <string>
#include <vector>

namespace SRE {
    class Texture;

    enum class UniformType {
        Int,
        Float,
        Mat3,
        Mat4,
        Vec4,
        Texture,
        Invalid
    };

    struct DllExport Uniform {
		char* name[50];
        int id;
        UniformType type;
        // 1 means not array
        int arrayCount;
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
     */
    class DllExport Shader {
    public:

        class DllExport ShaderBuilder {
        public:
            ShaderBuilder& withSource(const char* vertexShader, const char* fragmentShader);
            ShaderBuilder& withSourceStandard();
            ShaderBuilder& withSourceUnlit();
            ShaderBuilder& withSourceUnlitSprite();
            ShaderBuilder& withSourceStandardParticles();
            ShaderBuilder& withSourceDebugUV();
            ShaderBuilder& withSourceDebugNormals();
            ShaderBuilder& widthDepthTest(bool enable);
            ShaderBuilder& withDepthTest(bool enable);
            ShaderBuilder& withDepthWrite(bool enable);
            ShaderBuilder& withBlend(BlendType blendType);
            Shader* build();
        private:
            ShaderBuilder() = default;
            const char* vertexShaderStr;
            const char* fragmentShaderStr;
            bool depthTest = true;
            bool depthWrite = true;
            BlendType blend = BlendType::Disabled;
            friend class Shader;
        };

        // Phong Light Model. Uses light objects and ambient light set in SimpleRenderEngine.
        // Attributes
        // "color" vec4 (default (1,1,1,1))
        // "tex" Texture* (default white texture)
        // "specularity" float (default 0 = no specularity)
        static Shader *getStandard();
        // Unlit model.
        // Attributes
        // "color" vec4 (default (1,1,1,1))
        // "tex" Texture* (default white texture)
        static Shader *getUnlit();
        // UnlitSprite = no depth examples and alpha blending
        // Attributes
        // "color" vec4 (default (1,1,1,1))
        // "tex" Texture* (default white texture)
        static Shader *getUnlitSprite();
        // StandardParticles
        // Attributes
        // "tex" Texture* (default alpha sphere texture)
        static Shader *getStandardParticles();

        static ShaderBuilder create();

        ~Shader();

        bool contains(const char* name);

        Uniform getType(const char* name);

        bool set(const char *name, glm::mat4 value);
        bool set(const char *name, glm::mat3 value);
        bool set(const char *name, glm::vec4 value);
        bool set(const char *name, float value);
        bool set(const char *name, int value);
        /// textureSlot: If sampling multiple textures from a single shader, each texture must be bound to a unique texture slot
        bool set(const char *name, Texture* texture, unsigned int textureSlot = 0);

        bool isDepthTest();

        bool isDepthWrite();

        BlendType getBlend();

    private:
        bool setLights(Light value[4], glm::vec4 ambient, glm::mat4 viewTransform);

        Shader();
        bool build(const char *vertexShader, const char *fragmentShader);

        void bind();

        unsigned int shaderProgramId;
        bool depthTest = true;
        bool depthWrite = true;
        BlendType blend = BlendType::Disabled;

        std::vector<Uniform> uniforms;
        void updateUniforms();

        friend class Mesh;
        friend class SimpleRenderEngine;
    public:
        static void translateToGLSLES(std::string &source, bool vertexShader);
    };
}