//
// Created by morten on 31/07/16.
//

#pragma once
#include "glm/glm.hpp"
#include "Light.h"
#include "BlendType.h"

namespace SRE {
    class Texture;

    /// Controls the apperance of the rendered objects.
    class Shader {
    public:
        static Shader *createShader(const char *vertexShader, const char *fragmentShader);

        static Shader *getUnlit();
        static Shader *getDebugUV();
        static Shader *getDebugNormals();
        static Shader *getStandard();

        // static Shader *createUnlitTexture();

        // static Shader *createUnlitColorAlphaBlend();

        // static Shader *createUnlitTextureAlphaBlend();

        // static Shader *createSpecularColor();

        // static Shader *createSpecularTexture();

        ~Shader();

        bool setMatrix(const char *name, glm::mat4 value);
        bool setMatrix(const char *name, glm::mat3 value);

        bool setVector(const char *name, glm::vec4 value);

        bool setFloat(const char *name, float value);

        bool setInt(const char *name, int value);

        /// textureSlot: If sampling multiple textures from a single shader, each texture must be bound to a unique texture slot
        bool setTexture(const char *name, Texture* texture, unsigned int textureSlot = 0);

        void setDepthTest(bool enable);

        bool isDepthTest();

        void setDepthWrite(bool enable);

        bool isDepthWrite();

        BlendType getBlend();

        void setBlend(BlendType blendType);

    private:
        bool setLights(Light value[4], glm::vec4 ambient);

        static Shader *standard;
        static Shader *unlit;

        static Shader *debugUV;
        static Shader *debugNormals;

        // static Shader *createUnlitTexture();

        // static Shader *unlitColorAlphaBlend;

        // static Shader *createUnlitTextureAlphaBlend();

        // static Shader *createDiffuseColor();

        // static Shader *createDiffuseTexture();

        // static Shader *createSpecularColor();

        // static Shader *createSpecularTexture();

        Shader();

        void bind();

        unsigned int shaderProgramId;
        bool depthTest = true;
        bool depthWrite = true;
        BlendType blend = BlendType::Disabled;

        friend class Mesh;
        friend class SimpleRenderEngine;
    };
}