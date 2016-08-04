//
// Created by morten on 31/07/16.
//

#include "Shader.h"

#if defined(_WIN32)
#   define GLEW_STATIC
#   include <GL/glew.h>
#else
#   include <OpenGL/gl3.h>
#endif
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>


namespace SRE {
    Shader *Shader::unlitColor = nullptr;
    Shader *Shader::debugUV = nullptr;
    Shader *Shader::debugNormals = nullptr;

    namespace {
        void logCurrentCompileException(GLuint shader, GLenum type, const char* source){
            GLint logSize = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

            std::vector<char> errorLog((unsigned long) logSize);
            glGetShaderInfoLog(shader, logSize, &logSize, errorLog.data());

            std::string typeStr;
            switch (type){
                case GL_FRAGMENT_SHADER:
                    typeStr = "Fragment shader";
                    break;
//                case GL_shader:
//                    typeStr = "Incomplete shader";
//                    break;
//                case ShaderErrorType::Linker:
//                    typeStr = "Linker";
//                    break;
                case GL_VERTEX_SHADER:
                    typeStr = "Vertex shader";
                    break;

            }
            std::cerr<<(std::string{errorLog.data()}+"\n"+ typeStr +" error\n")<<std::endl;
        }

        GLuint compileShader(const char* source, GLenum type){
            GLuint shader = glCreateShader(type);
            GLint length = (GLint)strlen(source);
            glShaderSource(shader, 1, &source, &length);
            glCompileShader(shader);
            GLint success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE){
                logCurrentCompileException(shader, type, source);
            }
            return shader;
        }

        bool linkProgram(GLuint mShaderProgram){

            glBindFragDataLocation(mShaderProgram, 0, "fragColor");

            glLinkProgram(mShaderProgram);

            GLint  linked;
            glGetProgramiv(mShaderProgram, GL_LINK_STATUS, &linked );
            if (linked == GL_FALSE) {
                GLint  logSize;
                glGetProgramiv(mShaderProgram, GL_INFO_LOG_LENGTH, &logSize);
                std::vector<char> errorLog((size_t) logSize);
                glGetProgramInfoLog(mShaderProgram, logSize, NULL, errorLog.data() );

                std::cerr<<(errorLog.data())<<std::endl;
                return false;
            }
            return true;
        }
    }

    Shader *Shader::createShader(const char *vertexShader, const char *fragmentShader) {
        Shader *res = new Shader();

        std::vector<const char*> shaderSrc{vertexShader, fragmentShader};
        std::vector<GLenum> shaderTypes{GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
        for (int i=0;i<2;i++) {
            GLuint s = compileShader(shaderSrc[i],shaderTypes[i]);
            glAttachShader(res->shaderProgramId,  s);
        }

        // enforce layout
        std::string attributeNames[3] = {"position", "normal", "uv"};
        for (int i=0;i<3;i++) {
            glBindAttribLocation(res->shaderProgramId, i, attributeNames[i].c_str());
        }

        bool linked = linkProgram(res->shaderProgramId);
        if (!linked){
            delete res;
            return nullptr;
        }

        return res;
    }

    Shader::Shader() {
        shaderProgramId = glCreateProgram();
    }

    Shader::~Shader() {
        glDeleteShader(shaderProgramId);
    }

    bool Shader::setMatrix(const char *name, glm::mat4 value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
            return false;
        }
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        return true;
    }

    bool Shader::setMatrix(const char *name, glm::mat3 value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
            return false;
        }
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
        return true;
    }

    bool Shader::setVector(const char *name, glm::vec4 value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
            return false;
        }
        glUniform4fv(location, 1, glm::value_ptr(value));
        return true;
    }

    bool Shader::setFloat(const char *name, float value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
            return false;
        }
        glUniform1f(location, value);
        return true;
    }

    bool Shader::setInt(const char *name, int value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
            return false;
        }
        glUniform1i(location, value);
        return true;
    }

    bool Shader::setLights(Light value[4]){
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, "lights");
        if (location == -1) {
            return false;
        }

        std::cout << "Set light not implemented!" << std::endl;
        return true;

    }

    void Shader::bind() {
        glUseProgram(shaderProgramId);
        if (depthTest) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        glDepthMask(depthWrite ? GL_TRUE : GL_FALSE);
        switch (blending) {
            case BlendingType::Disabled:
                glDisable(GL_BLEND);
                break;
            case BlendingType::AlphaBlending:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                break;
        }
    }

    void Shader::setDepthTest(bool enable) {
        depthTest = enable;
    }

    bool Shader::isDepthTest() {
        return depthTest;
    }

    void Shader::setDepthWrite(bool enable) {
        depthWrite = enable;
    }

    bool Shader::isDepthWrite() {
        return depthWrite;
    }

    BlendingType Shader::getBlending() {
        return blending;
    }

    void Shader::setBlending(BlendingType blendingType) {
        blending = blendingType;
    }

    Shader *Shader::createUnlitColor() {
        if (unlitColor != nullptr){
            return unlitColor;
        }
        const char* vertexShader = R"(#version 330
in vec4 position;
in vec3 normal;
in vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void) {
    gl_Position = projection * view * model * position;
}
)";
        const char* fragmentShader = R"(#version 330
out vec4 fragColor;

uniform vec4 color;

void main(void)
{
    fragColor = color;
}
)";
        unlitColor = createShader(vertexShader, fragmentShader);
        return unlitColor;
    }

    Shader *Shader::createDebugUV() {
        if (debugUV != nullptr){
            return debugUV;
        }
        const char* vertexShader = R"(#version 330
in vec4 position;
in vec3 normal;
in vec2 uv;
out vec2 vUV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(void) {
    gl_Position = projection * view * model * position;
    vUV = uv;
}
)";
        const char* fragmentShader = R"(#version 330
in vec2 vUV;
out vec4 fragColor;

void main(void)
{
    fragColor = vec4(vUV,0.0,1.0);
}
)";
        debugUV = createShader(vertexShader, fragmentShader);
        return debugUV;
    }

    Shader *Shader::createDebugNormals() {
        if (debugNormals != nullptr){
            return debugNormals;
        }
        const char* vertexShader = R"(#version 330
in vec4 position;
in vec3 normal;
in vec2 uv;
out vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

void main(void) {
    gl_Position = projection * view * model * position;
    vNormal = normalMat * normal;
}
)";
        const char* fragmentShader = R"(#version 330
out vec4 fragColor;
in vec3 vNormal;

void main(void)
{
    fragColor = vec4(vNormal*0.5+0.5,1.0);
}
)";
        debugNormals = createShader(vertexShader, fragmentShader);
        return debugNormals;
    }
    Shader *Shader::createSpecularColor() {
        if (specularColor != nullptr){
            return specularColor;
        }
        const char* vertexShader = R"(#version 330
in vec4 position;
in vec3 normal;
in vec2 uv;
out vec3 vNormal;
uniform mat4 light[4];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

void main(void) {
    gl_Position = projection * view * model * position;
    vNormal = normalMat * normal;
}
)";
        const char* fragmentShader = R"(#version 330
out vec4 fragColor;
in vec3 vNormal;


void main(void)
{
    fragColor = vec4(vNormal*0.5+0.5,1.0);
}
)";
        specularColor = createShader(vertexShader, fragmentShader);
        return specularColor;
    }
}