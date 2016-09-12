//
// Created by morten on 31/07/16.
//

#pragma once
#include "glm/glm.hpp"
#include "Light.hpp"
#include "BlendType.hpp"

#include "Export.hpp"

namespace SRE {
    class Texture;

    /// Controls the apperance of the rendered objects.
    class DllExport Shader {
    public:
        static Shader *createShader(const char *vertexShader, const char *fragmentShader);

        static Shader *getStandard();
        static Shader *getUnlit();
        static Shader *getFont();

        static Shader *getDebugUV();
        static Shader *getDebugNormals();

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
        bool setLights(Light value[4], glm::vec4 ambient, glm::mat4 viewTransform);

        static Shader *standard;
        static Shader *unlit;
        static Shader *font;

        static Shader *debugUV;
        static Shader *debugNormals;

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