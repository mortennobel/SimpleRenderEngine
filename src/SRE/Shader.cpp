//
// Created by morten on 31/07/16.
//

#include "SRE/Shader.hpp"

#include "SRE/SREGL.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <vector>
#include "SRE/Texture.hpp"

namespace SRE {
    Shader *Shader::standard = nullptr;
    Shader *Shader::unlit = nullptr;
    Shader *Shader::unlitSprite = nullptr;
    Shader *Shader::font = nullptr;
    Shader *Shader::debugUV = nullptr;
    Shader *Shader::debugNormals = nullptr;

    namespace {
        void logCurrentCompileException(GLuint shader, GLenum type) {
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
                default:
                    typeStr = std::string("Unknown error type: ")+std::to_string(type);
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
                logCurrentCompileException(shader, type);
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

                std::cerr << (errorLog.data()) << std::endl;
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
        return set(name, value);
    }

    bool Shader::setMatrix(const char *name, glm::mat3 value) {
        return set(name, value);
    }

    bool Shader::setVector(const char *name, glm::vec4 value) {
        return set(name, value);
    }

    bool Shader::setFloat(const char *name, float value) {
        return set(name, value);
    }

    bool Shader::setInt(const char *name, int value) {
        return set(name, value);
    }

    bool Shader::setTexture(const char *name, Texture *texture, unsigned int textureSlot) {
        return set(name, texture, textureSlot);
    }

    bool Shader::set(const char *name, glm::mat4 value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
#ifdef DEBUG
            std::cout<<"Cannot find shader uniform "<<name<<endl;
#endif
            return false;
        }
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        return true;
    }

    bool Shader::set(const char *name, glm::mat3 value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
#ifdef DEBUG
            std::cout<<"Cannot find shader uniform "<<name<<endl;
#endif
            return false;
        }
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
        return true;
    }

    bool Shader::set(const char *name, glm::vec4 value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
#ifdef DEBUG
            std::cout<<"Cannot find shader uniform "<<name<<endl;
#endif
            return false;
        }
        glUniform4fv(location, 1, glm::value_ptr(value));
        return true;
    }

    bool Shader::set(const char *name, float value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
#ifdef DEBUG
            std::cout<<"Cannot find shader uniform "<<name<<endl;
#endif
            return false;
        }
        glUniform1f(location, value);
        return true;
    }

    bool Shader::set(const char *name, int value) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
#ifdef DEBUG
            std::cout<<"Cannot find shader uniform "<<name<<endl;
#endif
            return false;
        }
        glUniform1i(location, value);
        return true;
    }

    bool Shader::set(const char *name, Texture *texture, unsigned int textureSlot) {
        glUseProgram(shaderProgramId);
        GLint location = glGetUniformLocation(shaderProgramId, name);
        if (location == -1) {
#ifdef DEBUG
            std::cout<<"Cannot find shader uniform "<<name<<endl;
#endif
            return false;
        }

        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, texture->textureId);
        glUniform1i(location, textureSlot);
        return true;
    }

    bool Shader::setLights(Light value[4], glm::vec4 ambient, glm::mat4 viewTransform){
        glUseProgram(shaderProgramId);

        GLint location = glGetUniformLocation(shaderProgramId, "ambientLight");
        if (location == -1) {
            return false;
        }
        glUniform4fv(location, 1, glm::value_ptr(ambient));

        location = glGetUniformLocation(shaderProgramId, "lightPosType");
        GLint location2 = glGetUniformLocation(shaderProgramId, "lightColorRange");
        if (location == -1 || location2 == -1) {
            return false;
        }

        glm::vec4 lightPosType[4];
        glm::vec4 lightColorRange[4];
        for (int i=0;i<4;i++){
            if (value[i].lightType == LightType::Point){
                lightPosType[i] = glm::vec4(value[i].position, 1);
            } else if (value[i].lightType == LightType::Directional){
                lightPosType[i] = glm::vec4(value[i].direction, 0);
            } else if (value[i].lightType == LightType::Unused){
                lightPosType[i] = glm::vec4(value[i].direction, 2);
                continue;
            }
            // transform to eye space
            lightPosType[i] = viewTransform * lightPosType[i];
            lightColorRange[i] = glm::vec4(value[i].color, value[i].range);
        }
        glUniform4fv(location, 4, glm::value_ptr(lightPosType[0]));
        glUniform4fv(location2, 4, glm::value_ptr(lightColorRange[0]));

        return true;
    }

    void Shader::bind() {
        glUseProgram(shaderProgramId);
        if (depthTest) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }
        GLboolean dm = (GLboolean) (depthWrite ? GL_TRUE : GL_FALSE);
        glDepthMask(dm);
        switch (blend) {
            case BlendType::Disabled:
                glDisable(GL_BLEND);
                break;
            case BlendType::AlphaBlending:
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

    BlendType Shader::getBlend() {
        return blend;
    }

    void Shader::setBlend(BlendType blendType) {
        blend = blendType;
    }

    Shader *Shader::getUnlit() {
        if (unlit != nullptr){
            return unlit;
        }
        const char* vertexShader = R"(#version 140
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
        const char* fragmentShader = R"(#version 140
out vec4 fragColor;
in vec2 vUV;

uniform vec4 color;
uniform sampler2D tex;

void main(void)
{
    fragColor = color * texture(tex, vUV);
}
)";
        unlit = createShader(vertexShader, fragmentShader);
        unlit->set("color", glm::vec4(1));
        unlit->set("tex", Texture::getWhiteTexture());
        return unlit;
    }
    
    Shader *Shader::getUnlitSprite() {
        if (unlitSprite != nullptr){
            return unlitSprite;
        }
        const char* vertexShader = R"(#version 140
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
        const char* fragmentShader = R"(#version 140
        out vec4 fragColor;
        in vec2 vUV;
        
        uniform vec4 color;
        uniform sampler2D tex;
        
        void main(void)
        {
            fragColor = color * texture(tex, vUV);
        }
        )";
        unlitSprite = createShader(vertexShader, fragmentShader);
        unlitSprite->set("color", glm::vec4(1));
        unlitSprite->set("tex", Texture::getWhiteTexture());
        unlitSprite->setBlend(BlendType::AlphaBlending);
        unlitSprite->setDepthTest(false);
        return unlitSprite;
    }


Shader *Shader::getFont() {
    if (font != nullptr){
        return font;
    }
    const char* vertexShader = R"(#version 140
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
    const char* fragmentShader = R"(#version 140
out vec4 fragColor;
in vec2 vUV;

uniform vec4 color;
uniform sampler2D tex;

void main(void)
{
    fragColor = color * texture(tex, vUV);
}
)";
    unlit = createShader(vertexShader, fragmentShader);
    unlit->set("color", glm::vec4(1));
    unlit->set("tex", Texture::getFontTexture());
    unlit->setBlend(BlendType::AlphaBlending);
    return unlit;
}

    Shader *Shader::getDebugUV() {
        if (debugUV != nullptr){
            return debugUV;
        }
        const char* vertexShader = R"(#version 140
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
        const char* fragmentShader = R"(#version 140
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

    Shader *Shader::getDebugNormals() {
        if (debugNormals != nullptr){
            return debugNormals;
        }
        const char* vertexShader = R"(#version 140
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
        const char* fragmentShader = R"(#version 140
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
    Shader *Shader::getStandard() {
        if (standard != nullptr){
            return standard;
        }
        const char* vertexShader = R"(#version 140
in vec4 position;
in vec3 normal;
in vec2 uv;
out vec3 vNormal;
out vec2 vUV;
out vec3 vEyePos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMat;

void main(void) {
    vec4 eyePos = view * model * position;
    gl_Position = projection * eyePos;
    vNormal = normalMat * normal;
    vUV = uv;
    vEyePos = eyePos.xyz;
}
)";
        const char* fragmentShader = R"(#version 140
out vec4 fragColor;
in vec3 vNormal;
in vec2 vUV;
in vec3 vEyePos;

uniform vec4 ambientLight;
uniform vec4 color;
uniform sampler2D tex;

uniform vec4 lightPosType[4];
uniform vec4 lightColorRange[4];
uniform float specularity;

vec3 computeLight(){
    vec3 lightColor = ambientLight.xyz;
    vec3 normal = normalize(vNormal);

    float diffuseFrac = 1.0 - ambientLight.w;

    float diffuse = 0;
    float specular = 0;
    for (int i=0;i<4;i++){
        bool isDirectional = lightPosType[i].w == 0.0;
        bool isPoint       = lightPosType[i].w == 1.0;
        vec3 lightDirection;
        float att = 1.0;
        if (isDirectional){
            lightDirection = lightPosType[i].xyz;
        } else if (isPoint) {
            vec3 lightVector = lightPosType[i].xyz - vEyePos;
            float lightVectorLength = length(lightVector);
            lightDirection = lightVector/lightVectorLength;
            att = pow(1.0-1/lightColorRange[i].w,2.0); // non physical range based attenuation
        } else {
            continue;
        }
        vec3 H = normalize(lightDirection - vEyePos);
        // diffuse light
        float thisDiffuse = max(0.0,dot(lightDirection, normal));
        if (thisDiffuse > 0.0){
            lightColor += (att * diffuseFrac * thisDiffuse) * lightColorRange[i].xyz;
        }
        // specular light
        if (specularity > 0){
            float nDotHV = dot(normal, H);
            if (nDotHV > 0){
                float pf = pow(nDotHV, specularity);
                lightColor += vec3(att * diffuseFrac * diffuseFrac * pf); // white specular highlights
            }
        }
    }

    return lightColor;
}

void main(void)
{
    vec4 c = color * texture(tex, vUV);

    vec3 l = computeLight();

    fragColor = c * vec4(l, 1.0);
}
)";
        standard = createShader(vertexShader, fragmentShader);
        standard->set("color", glm::vec4(1));
        standard->set("specularity", 0);
        standard->set("tex", Texture::getWhiteTexture());
        return standard;
    }
}
