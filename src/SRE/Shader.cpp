//
// Created by morten on 31/07/16.
//

#include "SRE/Shader.hpp"

#include "SRE/impl/GL.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <regex>
#include <vector>
#include <map>
#include <sstream>
#include <SRE/SimpleRenderEngine.hpp>
#include "SRE/Texture.hpp"

namespace SRE {

    Shader *standard = nullptr;
    Shader *unlit = nullptr;
    Shader *unlitSprite = nullptr;
    Shader *standardParticles = nullptr;


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

        bool compileShader(std::string source, GLenum type, GLuint& shader){
#ifdef EMSCRIPTEN
            Shader::translateToGLSLES(source, type==GL_VERTEX_SHADER);
#endif
            shader = glCreateShader(type);
            GLint length = (GLint)strlen(source.c_str());
            auto stringPtr = source.c_str();
            glShaderSource(shader, 1, &stringPtr, &length);
            glCompileShader(shader);
            GLint success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (success == GL_FALSE){
                logCurrentCompileException(shader, type);
                return false;
            }
            return true;
        }

        bool linkProgram(GLuint mShaderProgram){
#ifndef EMSCRIPTEN
            glBindFragDataLocation(mShaderProgram, 0, "fragColor");
#endif
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

    void Shader::translateToGLSLES(std::string &source, bool vertexShader) {
        using namespace std;

        string replace = "#version 140";
        string replaceWith = vertexShader?"#version 100":"#version 100\n#ifdef GL_ES\n"
                "precision mediump float;\n"
                "#endif";
        size_t f = source.find(replace);
        source = source.replace(f, replace.length(), replaceWith);

        // replace textures
        if (vertexShader){
            static regex regExpSearchShim3 { R"(\n\s*out\b)"};
            source = regex_replace(source, regExpSearchShim3, "\nvarying");
            static regex regExpSearchShim4 { R"(\n\s*in\b)"};
            source = regex_replace(source, regExpSearchShim4, "\nattribute");
        } else {
            static regex regExpSearchShim2 { R"(\bfragColor\b)"};
            source = regex_replace(source, regExpSearchShim2, "gl_FragColor");
            static regex regExpSearchShim3 { R"(\n\s*out\b)"};
            source = regex_replace(source, regExpSearchShim3, "\n// out");
            static regex regExpSearchShim4 { R"(\n\s*in\b)"};
            source = regex_replace(source, regExpSearchShim4, "\nvarying");
        }

        static regex regExpSearchShim1 { R"(\s*uniform\s+sampler([\w]*)\s+([\w_]*)\s*;.*)"};
        istringstream iss(source);
        map<string,string> textureType;
        smatch match;
        for (std::string line; std::getline(iss, line); )
        {
            regex_search(line, match, regExpSearchShim1);
            if (match.size() > 0){
                string samplerType = match[1].str();
                string samplerName = match[2].str();
                textureType[samplerName] = samplerType;

            }
        }

        for (auto val : textureType){
            regex regExpSearchShim4 { string{"texture\\s*\\(\\s*"}+val.first+"\\s*," };
            source = regex_replace(source, regExpSearchShim4, string{"texture"}+val.second+"("+val.first+",");
        }
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
			Uniform u;
			memcpy(u.name, name, 50);
			u.id = location;
			u.arrayCount = size;
			u.type = uniformType;
            uniforms.push_back(u);
        }
    }

    Shader::Shader() {
        shaderProgramId = glCreateProgram();
        SimpleRenderEngine::instance->renderStats.shaderCount++;
    }

    Shader::~Shader() {
        glDeleteShader(shaderProgramId);
        SimpleRenderEngine::instance->renderStats.shaderCount--;
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
        glBindTexture(texture->target, texture->textureId);

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
    }

    bool Shader::isDepthTest() {
        return depthTest;
    }

    bool Shader::isDepthWrite() {
        return depthWrite;
    }

    BlendType Shader::getBlend() {
        return blend;
    }

    Shader *Shader::getUnlit() {
        if (unlit != nullptr){
            return unlit;
        }

        unlit = create().withSourceUnlit().build();
        unlit->set("color", glm::vec4(1));
        unlit->set("tex", Texture::getWhiteTexture());
        return unlit;
    }
    
    Shader *Shader::getUnlitSprite() {
        if (unlitSprite != nullptr){
            return unlitSprite;
        }

        unlitSprite =  create()
                .withSourceUnlitSprite()
                .withBlend(BlendType::AlphaBlending)
                .withDepthTest(false)
                .build();
        unlitSprite->set("color", glm::vec4(1));
        unlitSprite->set("tex", Texture::getWhiteTexture());
        return unlitSprite;
    }


    Shader *Shader::getStandard() {
        if (standard != nullptr){
            return standard;
        }
        standard = create().withSourceStandard().build();
        standard->set("color", glm::vec4(1));
        standard->set("specularity", 0.0f);
        standard->set("tex", Texture::getWhiteTexture());
        return standard;
    }

    bool Shader::contains(const char *name) {
		for (auto i = uniforms.begin(); i != uniforms.end(); i++) {
			if (strcmp((const char*)i->name, name) == 0)
				return true;
		}
        return false;
    }

    Uniform Shader::getType(const char *name) {
		for (auto i = uniforms.begin(); i != uniforms.end(); i++) {
			if (strcmp((const char*)i->name, name) == 0)
				return *i;
		}
		Uniform u;
		u.type = UniformType::Invalid;
		u.id = -1;
		u.arrayCount = -1;
		return u;
    }

    Shader *Shader::getStandardParticles() {
        if (standardParticles != nullptr){
            return standardParticles;
        }

        standardParticles = create()
                .withSourceStandardParticles()
                .withBlend(BlendType::AdditiveBlending)
                .withDepthWrite(false)
                .build();
        standardParticles->set("tex", Texture::getSphereTexture());
        return standardParticles;
    }

    Shader::ShaderBuilder Shader::create() {
        return Shader::ShaderBuilder();
    }

    bool Shader::build(const char *vertexShader, const char *fragmentShader) {
        std::vector<const char*> shaderSrc{vertexShader, fragmentShader};
        std::vector<GLenum> shaderTypes{GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
        for (int i=0;i<2;i++) {
            GLuint s;
            bool res = compileShader(shaderSrc[i],shaderTypes[i], s);
            if (!res){
                return false;
            }
            glAttachShader(shaderProgramId,  s);
        }

        // enforce layout
        std::string attributeNames[4] = {"position", "normal", "uv", "color"};
        for (int i=0;i<4;i++) {
            glBindAttribLocation(shaderProgramId, i, attributeNames[i].c_str());
        }

        bool linked = linkProgram(shaderProgramId);
        if (!linked){
            return false;
        }
        updateUniforms();
        return true;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSource(const char *vertexShader, const char *fragmentShader) {
        this->vertexShaderStr = vertexShader;
        this->fragmentShaderStr = fragmentShader;
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceStandard() {
        this->vertexShaderStr = R"(#version 140
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
        this->fragmentShaderStr = R"(#version 140
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
            lightDirection = normalize(lightPosType[i].xyz);
        } else if (isPoint) {
            vec3 lightVector = lightPosType[i].xyz - vEyePos;
            float lightVectorLength = length(lightVector);
            float lightRange = lightColorRange[i].w;
            lightDirection = lightVector / lightVectorLength;
            if (lightRange <= 0.0){
                att = 1.0;
            } else if (lightVectorLength >= lightRange){
                att = 0.0;
            } else {
                att = pow(1.0 - lightVectorLength / lightRange,1.5); // non physical range based attenuation
            }
        } else {
            continue;
        }

        // diffuse light
        float thisDiffuse = max(0.0,dot(lightDirection, normal));
        if (thisDiffuse > 0.0){
           lightColor += (att * diffuseFrac * thisDiffuse) * lightColorRange[i].xyz;
        }

        // specular light
        if (specularity > 0.0){
            vec3 H = normalize(lightDirection - normalize(vEyePos));
            float nDotHV = dot(normal, H);
            if (nDotHV > 0.0){
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
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceUnlit() {
        this->vertexShaderStr = R"(#version 140
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
        this->fragmentShaderStr = R"(#version 140
out vec4 fragColor;
in vec2 vUV;

uniform vec4 color;
uniform sampler2D tex;

void main(void)
{
    fragColor = color * texture(tex, vUV);
}
)";
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceUnlitSprite() {
        this->vertexShaderStr = R"(#version 140
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
        this->fragmentShaderStr = R"(#version 140
        out vec4 fragColor;
        in vec2 vUV;

        uniform vec4 color;
        uniform sampler2D tex;

        void main(void)
        {
            fragColor = color * texture(tex, vUV);
        }
        )";
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceStandardParticles() {
        this->vertexShaderStr = R"(#version 140
in vec4 position;
in vec3 normal;
in vec4 uv;
in vec4 color;
out mat3 vUVMat;
out vec4 vColor;
out vec3 uvSize;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float view_height;

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
    if (projection[2][3] != 0.0){ // if perspective projection
        vec3 ndc = gl_Position.xyz / gl_Position.w ; // perspective divide.


        float zDist = 1.0-ndc.z ; // 1 is close (right up in your face,)
        if (zDist < 0.0 || zDist > 1.0)
        {
            zDist = 0.0;
        }
        gl_PointSize = view_height * position.w * zDist;
    } else {
        gl_PointSize = 0.1 * view_height * position.w;
    }

    vUVMat = translate(uv.xy)*scale(uv.z) * translate(vec2(0.5,0.5))*rotate(uv.w) * translate(vec2(-0.5,-0.5));
    vColor = color;
    uvSize = uv.xyz;
}
)";
        this->fragmentShaderStr = R"(#version 140
out vec4 fragColor;
in mat3 vUVMat;
in vec3 uvSize;
in vec4 vColor;

uniform sampler2D tex;

void main(void)
{
    vec2 uv = (vUVMat * vec3(gl_PointCoord,1.0)).xy;

    if (uv != clamp(uv, uvSize.xy, uvSize.xy + uvSize.zz)){
        discard;
    }
    vec4 c = vColor * texture(tex, uv);
    fragColor = c;
}
)";
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceDebugUV() {
        this->vertexShaderStr = R"(#version 140
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
        this->fragmentShaderStr = R"(#version 140
in vec2 vUV;
out vec4 fragColor;

void main(void)
{
    fragColor = vec4(vUV,0.0,1.0);
}
)";
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceDebugNormals() {
        this->vertexShaderStr = R"(#version 140
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
        this->fragmentShaderStr = R"(#version 140
out vec4 fragColor;
in vec3 vNormal;

void main(void)
{
    fragColor = vec4(vNormal*0.5+0.5,1.0);
}
)";
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withDepthTest(bool enable) {

        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::widthDepthTest(bool enable) {
        this->depthTest = enable;
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withDepthWrite(bool enable) {
        this->depthWrite = enable;
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withBlend(BlendType blendType) {
        this->blend = blendType;
        return *this;
    }

    Shader *Shader::ShaderBuilder::build() {
        Shader* res = new Shader();
        if (!res->build(vertexShaderStr, fragmentShaderStr)){
            delete res;
            return nullptr;
        }
        res->depthTest = this->depthTest;
        res->depthWrite = this->depthWrite;
        res->blend = this->blend;
        return res;
    }
}
