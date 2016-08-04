//
// Created by morten on 31/07/16.
//

#pragma once
#include "glm/glm.hpp"
#include "Light.h"
#include "BlendType.h"

namespace SRE {
    class Shader {
    public:
        static Shader *createShader(const char *vertexShader, const char *fragmentShader);

        static Shader *createUnlitColor();
        static Shader *createDebugUV();
        static Shader *createDebugNormals();
        static Shader *createSpecularColor();

        // static Shader *createUnlitTexure();

        // static Shader *createUnlitColorAlphaBlend();

        // static Shader *createUnlitTexureAlphaBlend();

        // static Shader *createSpecularColor();

        // static Shader *createSpecularTexture();

        ~Shader();

        bool setMatrix(const char *name, glm::mat4 value);
        bool setMatrix(const char *name, glm::mat3 value);

        bool setVector(const char *name, glm::vec4 value);

        bool setFloat(const char *name, float value);

        bool setInt(const char *name, int value);

        void setDepthTest(bool enable);

        bool isDepthTest();

        void setDepthWrite(bool enable);

        bool isDepthWrite();

        BlendType getBlend();

        void setBlend(BlendType blendType);

    private:
        bool setLights(Light value[4]);

        static Shader *unlitColor;

        static Shader *debugUV;

        static Shader *debugNormals;
        static Shader *specularColor;

        // static Shader *createUnlitTexure();

        // static Shader *unlitColorAlphaBlend;

        // static Shader *createUnlitTexureAlphaBlend();

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