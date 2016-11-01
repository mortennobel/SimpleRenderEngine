//
// Created by morten on 31/07/16.
//

#include "SRE/Shader.hpp"

#include "SRE/GL.hpp"
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
    Shader *Shader::standardParticles = nullptr;
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

    Shader *Shader::createShader(const char *vertexShader, const char *fragmentShader, bool particleLayout) {
        Shader *res = new Shader();

        std::vector<const char*> shaderSrc{vertexShader, fragmentShader};
        std::vector<GLenum> shaderTypes{GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
        for (int i=0;i<2;i++) {
            GLuint s = compileShader(shaderSrc[i],shaderTypes[i]);
            glAttachShader(res->shaderProgramId,  s);
        }

        // enforce layout
        std::string attributeNames[3] = {"position", particleLayout?"color":"normal", "uv"};
        for (int i=0;i<3;i++) {
            glBindAttribLocation(res->shaderProgramId, i, attributeNames[i].c_str());
        }

        bool linked = linkProgram(res->shaderProgramId);
        if (!linked){
            delete res;
            return nullptr;
        }

        res->updateUniforms();
        res->particleLayout = particleLayout;
        return res;
    }

    void Shader::updateUniforms() {
        uniforms.clear();
        GLint uniformCount;
        glGetProgramiv(shaderProgramId,GL_ACTIVE_UNIFORMS,&uniformCount);
        UniformType uniformType = UniformType::Invalid;
        for (int i=0;i<uniformCount;i++){
            const int nameSize = 50;
            GLchar name[nameSize];
            GLsizei nameLength;
            GLint size;
            GLenum type;
            glGetActiveUniform(	shaderProgramId,
                    i,
                    nameSize,
                    &nameLength,
                    &size,
                    &type,
                    name);
            switch (type){
                case GL_FLOAT:
                    uniformType = UniformType::Float;
                    break;
                case GL_FLOAT_VEC4:
                    uniformType = UniformType::Vec4;
                    break;
                case GL_INT:
                    uniformType = UniformType::Int;
                    break;
                case GL_FLOAT_MAT3:
                    uniformType = UniformType::Mat3;
                    break;
                case GL_FLOAT_MAT4:
                    uniformType = UniformType::Mat4;
                    break;
                case GL_SAMPLER_2D:
                    uniformType = UniformType::Texture;
                    break;
                default:
                std::cerr << "Unsupported shader type "<<type<<" name "<<name<<std::endl;
            }
            // remove [0] if exists
            char *bracketIndex = strchr(name, '[');
            if (bracketIndex != nullptr){
                *bracketIndex = '\0';
            }
            GLint location = glGetUniformLocation(shaderProgramId, name);
            uniforms[name] = {location,uniformType,size};
        }
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
        auto uniform = getType(name);
        if (uniform.id == -1) {
#ifndef NDEBUG
            std::cerr<<"Cannot find shader uniform "<<name<<std::endl;
#endif
            return false;
        }
#ifndef NDEBUG
        if (uniform.type != UniformType::Mat4){
            std::cerr << "Invalid shader uniform type for "<<name<<std::endl;
        }
        if (uniform.arrayCount != 1){
            std::cerr << "Invalid shader uniform array count for "<<name<<std::endl;
        }
#endif
        glUniformMatrix4fv(uniform.id, 1, GL_FALSE, glm::value_ptr(value));
        return true;
    }

    bool Shader::set(const char *name, glm::mat3 value) {
        glUseProgram(shaderProgramId);
        auto uniform = getType(name);
        if (uniform.id == -1) {
#ifndef NDEBUG
            std::cout<<"Cannot find shader uniform "<<name<<std::endl;
#endif
            return false;
        }
#ifndef NDEBUG
        if (uniform.type != UniformType::Mat3){
            std::cerr << "Invalid shader uniform type for "<<name<<std::endl;
        }
        if (uniform.arrayCount != 1){
            std::cerr << "Invalid shader uniform array count for "<<name<<std::endl;
        }
#endif
        glUniformMatrix3fv(uniform.id, 1, GL_FALSE, glm::value_ptr(value));
        return true;
    }

    bool Shader::set(const char *name, glm::vec4 value) {
        glUseProgram(shaderProgramId);
        auto uniform = getType(name);
        if (uniform.id == -1) {
#ifndef NDEBUG
            std::cout<<"Cannot find shader uniform "<<name<<std::endl;
#endif
            return false;
        }
#ifndef NDEBUG
        if (uniform.type != UniformType::Vec4){
            std::cerr << "Invalid shader uniform type for "<<name<<std::endl;
        }
        if (uniform.arrayCount != 1){
            std::cerr << "Invalid shader uniform array count for "<<name<<std::endl;
        }
#endif
        glUniform4fv(uniform.id, 1, glm::value_ptr(value));
        return true;
    }

    bool Shader::set(const char *name, float value) {
        glUseProgram(shaderProgramId);

        auto uniform = getType(name);
        if (uniform.id == -1) {
#ifndef NDEBUG
            std::cout<<"Cannot find shader uniform "<<name<<std::endl;
#endif
            return false;
        }
#ifndef NDEBUG
        if (uniform.type != UniformType::Float){
            std::cerr << "Invalid shader uniform type for "<<name<<std::endl;
        }
        if (uniform.arrayCount != 1){
            std::cerr << "Invalid shader uniform array count for "<<name<<std::endl;
        }
#endif
        glUniform1f(uniform.id, value);
        return true;
    }

    bool Shader::set(const char *name, int value) {
        glUseProgram(shaderProgramId);
        auto uniform = getType(name);
        if (uniform.id == -1) {
#ifndef NDEBUG
            std::cout<<"Cannot find shader uniform "<<name<<std::endl;
#endif
            return false;
        }
#ifndef NDEBUG
        if (uniform.type != UniformType::Int){
            std::cerr << "Invalid shader uniform type for "<<name << std::endl;
        }
        if (uniform.arrayCount != 1){
            std::cerr << "Invalid shader uniform array count for "<<name<<std::endl;
        }
#endif
        glUniform1i(uniform.id, value);
        return true;
    }

    bool Shader::set(const char *name, Texture *texture, unsigned int textureSlot) {
        glUseProgram(shaderProgramId);
        auto uniform = getType(name);
        if (uniform.id == -1) {
#ifndef NDEBUG
            std::cout<<"Cannot find shader uniform "<<name<<std::endl;
#endif
            return false;
        }
#ifndef NDEBUG
        if (uniform.type != UniformType::Texture){
            std::cerr << "Invalid shader uniform type for "<<name <<std::endl;
        }
        if (uniform.arrayCount != 1){
            std::cerr << "Invalid shader uniform array count for "<<name << std::endl;
        }
#endif
        glActiveTexture(GL_TEXTURE0 + textureSlot);
        glBindTexture(GL_TEXTURE_2D, texture->textureId);
        textureMap[textureSlot] = texture->textureId;
        glUniform1i(uniform.id, textureSlot);
        return true;
    }

    bool Shader::setLights(Light value[4], glm::vec4 ambient, glm::mat4 viewTransform){
        glUseProgram(shaderProgramId);

        auto uniform = getType("ambientLight");
        if (uniform.id != -1) {
            glUniform4fv(uniform.id, 1, glm::value_ptr(ambient));
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
        uniform = getType("lightPosType");
        if (uniform.id != -1) {
            if (uniform.arrayCount != 4){
                std::cerr << "Invalid shader uniform array count for lightPosType"<<std::endl;
            }
            glUniform4fv(uniform.id, 4, glm::value_ptr(lightPosType[0]));
        }
        uniform = getType("lightColorRange");
        if (uniform.id != -1) {
            if (uniform.arrayCount != 4){
                std::cerr << "Invalid shader uniform array count for lightColorRange"<<std::endl;
            }
            glUniform4fv(uniform.id, 4, glm::value_ptr(lightColorRange[0]));
        }
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
            case BlendType::AdditiveBlending:
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
                break;
            default:
                std::cout << "Err";
                break;
        }
        for (auto e : textureMap){
            glActiveTexture(GL_TEXTURE0 + e.first);
            glBindTexture(GL_TEXTURE_2D, e.second);
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
                lightColor += vec3(att * diffuseFrac * pf); // white specular highlights
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
        standard->set("specularity", 0.0f);
        standard->set("tex", Texture::getWhiteTexture());
        return standard;
    }

    bool Shader::contains(const char *name) {
        return uniforms.find(name) != uniforms.end();
    }

    Uniform Shader::getType(const char *name) {
        auto res = uniforms.find(name);
        if (res != uniforms.end()){
            return uniforms[name];
        } else {
            return {-1, UniformType::Invalid, -1};
        }
    }

    Shader *Shader::getStandardParticles() {
        if (standardParticles != nullptr){
            return standardParticles;
        }
        const char* vertexShader = R"(#version 140
in vec4 position;
in vec4 color;
in vec4 uv;
out mat3 vUVMat;
out vec4 vColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

mat3 translate(vec2 p){
 return mat3(1.0,0.0,0.0,0.0,1.0,0.0,p.x,p.y,1.0);
}

mat3 rotate(float rad){
  float s = sin(rad);
  float c = cos(rad);
 return mat3(c,s,0.0,-s,c,0.0,0.0,0.0,1.0);
}

mat3 scale(float s){
  return mat3(s,0.0,0.0,0.0,s,0.0,0.0,0.0,1.0);
}

void main(void) {
    vec4 pos = vec4( position.xyz, 1.0);
    gl_Position = projection * view * model * pos;
    if (projection[2][3] != 0){ // if perspective projection
        vec3 ndc = gl_Position.xyz / gl_Position.w ; // perspective divide.


        float zDist = 1.0-ndc.z ; // 1 is close (right up in your face,)
        if (zDist < 0.0 || zDist > 1.0)
        {
            zDist = 0.0;
        }
        gl_PointSize = position.w * zDist;
    } else {
        gl_PointSize = position.w * projection[0][0] * projection[1][1] * 0.5; // average x,y scale in orthographic projection
    }

    vUVMat = translate(uv.xy)*scale(uv.z) * translate(vec2(0.5,0.5))*rotate(uv.w) * translate(vec2(-0.5,-0.5));
    vColor = color;
}
)";
        const char* fragmentShader = R"(#version 140
out vec4 fragColor;
in mat3 vUVMat;
in vec4 vColor;

uniform sampler2D tex;

void main(void)
{
    vec2 uv = (vUVMat * vec3(gl_PointCoord,1.0)).xy;
    vec4 c = vColor * texture(tex, uv);
    fragColor = c;
}
)";
        standardParticles = createShader(vertexShader, fragmentShader, true);
        standardParticles->set("tex", Texture::getSphereTexture());
        standardParticles->setBlend(BlendType::AdditiveBlending);
        standardParticles->setDepthWrite(false);
        return standardParticles;
    }
}
