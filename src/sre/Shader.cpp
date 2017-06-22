//
// Created by morten on 31/07/16.
//

#include "sre/Shader.hpp"

#include "sre/impl/GL.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <regex>
#include <vector>
#include <map>
#include <sstream>
#include "sre/Renderer.hpp"
#include "sre/Texture.hpp"

namespace sre {


    namespace {
        Shader *standard = nullptr;
        Shader *unlit = nullptr;
        Shader *unlitSprite = nullptr;
        Shader *standardParticles = nullptr;

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
                case GL_VERTEX_SHADER:
                    typeStr = "Vertex shader";
                    break;
#ifndef EMSCRIPTEN
                case GL_GEOMETRY_SHADER:
                    typeStr = "Geometry shader";
                    break;
                case GL_TESS_CONTROL_SHADER:
                    typeStr = "Tessellation control shader";
                    break;
                case GL_TESS_EVALUATION_SHADER:
                    typeStr = "Tessellation eval shader";
                    break;
#endif
                default:
                    typeStr = std::string("Unknown error type: ")+std::to_string(type);
                    break;
            }
            std::cerr<<(std::string{errorLog.data()}+"\n"+ typeStr +" error\n")<<std::endl;
        }

        bool compileShader(std::string& source, GLenum type, GLuint& shader){
#ifdef EMSCRIPTEN
            Shader::translateToGLSLES(source, type==GL_VERTEX_SHADER);
#endif
            shader = glCreateShader(type);
            auto stringPtr = source.c_str();
            GLint length = (GLint)strlen(stringPtr);
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
        uniformLocationModel = -1;
        uniformLocationView = -1;
        uniformLocationProjection = -1;
        uniformLocationNormal = -1;
        uniformLocationViewport = -1;
        uniformLocationAmbientLight = -1;
        uniformLocationLightPosType = -1;
        uniformLocationLightColorRange = -1;
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
                case GL_SAMPLER_CUBE:
                    uniformType = UniformType::TextureCube;
                    break;

                default:
                std::cerr << "Unsupported shader type "<<type<<" name "<<name<<std::endl;
            }
            // remove [0] if exists
            char *bracketIndex = strchr(name, '[');
            if (bracketIndex != nullptr){
                *bracketIndex = '\0';
            }
            bool isGlobalUniform = name[0] == 'g' && name[1] == '_';
            GLint location = glGetUniformLocation(shaderProgramId, name);
            if (!isGlobalUniform){
                Uniform u;
                u.name = name;
                u.id = location;
                u.elementCount = size;
                u.type = uniformType;
                uniforms.push_back(u);
            } else {
                if (strcmp(name, "g_model")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationModel = location;
                    } else {
                        std::cerr << "Invalid g_model uniform type. Expected mat4.";
                    }
                }
                if (strcmp(name, "g_view")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationView = location;
                    } else {
                        std::cerr << "Invalid g_view uniform type. Expected mat4.";
                    }
                }
                if (strcmp(name, "g_projection")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationProjection = location;
                    } else {
                        std::cerr << "Invalid g_projection uniform type. Expected mat4.";
                    }
                }
                if (strcmp(name, "g_normal")==0){
                    if (uniformType == UniformType::Mat3){
                        uniformLocationNormal = location;
                    } else {
                        std::cerr << "Invalid g_normal uniform type. Expected mat3.";
                    }
                }
                if (strcmp(name, "g_viewport")==0){
                    if (uniformType == UniformType::Vec4){
                        uniformLocationViewport = location;
                    } else {
                        std::cerr << "Invalid g_normal uniform type. Expected vec4.";
                    }
                }
                if (strcmp(name, "g_ambientLight")==0){
                    if (uniformType == UniformType::Vec4){
                        uniformLocationAmbientLight = location;
                    } else {
                        std::cerr << "Invalid g_ambientLight uniform type. Expected vec4.";
                    }
                }
                if (strcmp(name, "g_lightPosType")==0){
                    if (uniformType == UniformType::Vec4 && size == 4){
                        uniformLocationLightPosType = location;
                    } else {
                        std::cerr << "Invalid g_lightPosType uniform type. Expected vec4[4].";
                    }
                }
                if (strcmp(name, "g_lightColorRange")==0){
                    if (uniformType == UniformType::Vec4 && size == 4){
                        uniformLocationLightColorRange = location;
                    } else {
                        std::cerr << "Invalid g_lightPosType uniform type. Expected vec4[4].";
                    }
                }
            }
        }
    }

    Shader::Shader() {
        shaderProgramId = glCreateProgram();
        Renderer::instance->renderStats.shaderCount++;
    }

    Shader::~Shader() {
        glDeleteShader(shaderProgramId);
        Renderer::instance->renderStats.shaderCount--;
    }

    bool Shader::setLights(WorldLights* worldLights, glm::mat4 viewTransform){
        if (worldLights==nullptr){
            glUniform4f(uniformLocationAmbientLight, 0,0,0,0);
            static float noLight[4*4] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
            glUniform4fv(uniformLocationLightPosType, 4, noLight);
            glUniform4fv(uniformLocationLightColorRange, 4, noLight);
            return false;
        }
        if (uniformLocationAmbientLight != -1) {
            glUniform4fv(uniformLocationAmbientLight, 1, glm::value_ptr(worldLights->ambientLight));
        }
        if (uniformLocationLightPosType != -1 && uniformLocationLightColorRange != -1){
            glm::vec4 lightPosType[4];
            glm::vec4 lightColorRange[4];
            for (int i=0;i<4;i++){
                auto light = worldLights == nullptr?nullptr:worldLights->getLight(i);
                if (light == nullptr || light->lightType == LightType::Unused) {
                    lightPosType[i] = glm::vec4(0.0f,0.0f,0.0f, 2);
                    continue;
                } else if (light->lightType == LightType::Point) {
                    lightPosType[i] = glm::vec4(light->position, 1);
                } else if (light->lightType == LightType::Directional) {
                    lightPosType[i] = glm::vec4(light->direction, 0);
                }
                // transform to eye space
                lightPosType[i] = viewTransform * lightPosType[i];
                lightColorRange[i] = glm::vec4(light->color, light->range);
            }
            if (uniformLocationLightPosType != -1) {
                glUniform4fv(uniformLocationLightPosType, 4, glm::value_ptr(lightPosType[0]));
            }
            if (uniformLocationLightColorRange != -1) {
                glUniform4fv(uniformLocationLightColorRange, 4, glm::value_ptr(lightColorRange[0]));
            }
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
        return unlitSprite;
    }


    Shader *Shader::getStandard() {
        if (standard != nullptr){
            return standard;
        }
        standard = create().withSourceStandard().build();
        return standard;
    }

    bool Shader::contains(const std::string& name) {
		for (auto i = uniforms.begin(); i != uniforms.end(); i++) {
			if (i->name.compare(name) == 0)
				return true;
		}
        return false;
    }

    Uniform Shader::getType(const std::string& name) {
		for (auto i = uniforms.begin(); i != uniforms.end(); i++) {
			if (i->name.compare(name) == 0)
				return *i;
		}
		Uniform u;
		u.type = UniformType::Invalid;
		u.id = -1;
		u.elementCount = -1;
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
        return standardParticles;
    }

    Shader::ShaderBuilder Shader::create() {
        return Shader::ShaderBuilder();
    }

    bool Shader::build(const std::string& vertexShader, const std::string& fragmentShader) {
        std::vector<std::string> shaderSrc{vertexShader, fragmentShader};
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

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSource(const std::string& vertexShader, const std::string& fragmentShader) {
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

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_normal;

void main(void) {
    vec4 eyePos = g_view * g_model * position;
    gl_Position = g_projection * eyePos;
    vNormal = normalize(g_normal * normal);
    vUV = uv;
    vEyePos = eyePos.xyz;
}
)";
        this->fragmentShaderStr = R"(#version 140
out vec4 fragColor;
in vec3 vNormal;
in vec2 vUV;
in vec3 vEyePos;

uniform vec4 g_ambientLight;
uniform vec4 color;
uniform sampler2D tex;

uniform vec4 g_lightPosType[4];
uniform vec4 g_lightColorRange[4];
uniform float specularity;

vec3 computeLight(){
    vec3 lightColor = g_ambientLight.xyz;
    vec3 normal = normalize(vNormal);

    float diffuseFrac = 1.0 - g_ambientLight.w;

    for (int i=0;i<4;i++){
        bool isDirectional = g_lightPosType[i].w == 0.0;
        bool isPoint       = g_lightPosType[i].w == 1.0;
        vec3 lightDirection;
        float att = 1.0;
        if (isDirectional){
            lightDirection = g_lightPosType[i].xyz;
        } else if (isPoint) {
            vec3 lightVector = g_lightPosType[i].xyz - vEyePos;
            float lightVectorLength = length(lightVector);
            float lightRange = g_lightColorRange[i].w;
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
           lightColor += (att * diffuseFrac * thisDiffuse) * g_lightColorRange[i].xyz;
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

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * position;
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

        uniform mat4 g_model;
        uniform mat4 g_view;
        uniform mat4 g_projection;

        void main(void) {
            gl_Position = g_projection * g_view * g_model * position;
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

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform vec4 g_viewport;

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
    gl_Position = g_projection * g_view * g_model * pos;
    if (g_projection[2][3] != 0.0){ // if perspective projection
        vec3 ndc = gl_Position.xyz / gl_Position.w ; // perspective divide.


        float zDist = 1.0-ndc.z ; // 1 is close (right up in your face,)
        if (zDist < 0.0 || zDist > 1.0)
        {
            zDist = 0.0;
        }
        gl_PointSize = g_viewport.y * position.w * zDist;
    } else {
        gl_PointSize = 0.1 * g_viewport.y * position.w;
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

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;

void main(void) {
    gl_Position = g_projection * g_view * g_model * position;
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

uniform mat4 g_model;
uniform mat4 g_view;
uniform mat4 g_projection;
uniform mat3 g_normal;

void main(void) {
    gl_Position = g_projection * g_view * g_model * position;
    vNormal = g_normal * normal;
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
