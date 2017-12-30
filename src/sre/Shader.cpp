/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include "sre/Shader.hpp"
#include "sre/Material.hpp"
#include "sre/impl/ShaderSource.inl"

#include "sre/impl/GL.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <cerrno>
#include <cctype>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <string>
#include <regex>
#include <utility>
#include <vector>
#include <map>
#include <sstream>
#include <utility>
#include <sre/Log.hpp>
#include "sre/Renderer.hpp"
#include "sre/Texture.hpp"

using namespace std;

namespace sre {
    // anonymous (file local) namespace
    namespace {
        std::shared_ptr<Shader> standard;
        std::shared_ptr<Shader> unlit;
        std::shared_ptr<Shader> unlitSprite;
        std::shared_ptr<Shader> standardParticles;

        // From https://stackoverflow.com/a/8473603/420250
        template <typename Map>
        bool map_compare (Map const &lhs, Map const &rhs) {
            // No predicate needed because there is operator== for pairs already.
            return lhs.size() == rhs.size()
                   && std::equal(lhs.begin(), lhs.end(),
                                 rhs.begin());
        }

        // from http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
        string getFileContents(string filename)
        {
            ifstream in{filename, ios::in | ios::binary};
            if (in && in.is_open())
            {
                std::string contents;
                in.seekg(0, std::ios::end);
                auto size = in.tellg();
                if (size>0){
                    contents.resize((string::size_type)size);
                    in.seekg(0, std::ios::beg);
                    in.read(&contents[0], contents.size());
                }
                in.close();
                return contents;
            }
            auto res = builtInShaderSource.find(filename);
            if (res != builtInShaderSource.end()){
                return res->second;
            }
            LOG_ERROR("Cannot find shader source %s", filename.c_str());
            return "";
        }

        std::string pragmaInclude(std::string source, std::vector<std::string>& errors, uint32_t shaderType){
            if (source.find("#pragma include")==-1) {
                return source;
            }
            std::stringstream sstream;
            auto pch = strtok (const_cast<char *>(source.c_str()), "\n\r"); // note destroys input string
            std::regex e ( R"_(#pragma\s+include\s+"([^"]*)")_", std::regex::ECMAScript);
            int lineNumber = 0;
            while (pch != nullptr)
            {
                lineNumber++;
                std::string s (pch);
                std::smatch m;
                if (std::regex_search (s,m,e)) {
                    std::string match = m[1];
                    auto res = getFileContents(match);
                    if (res == ""){
                        errors.push_back(std::string("0:")+std::to_string(lineNumber)+" cannot find include file "+match+"##"+std::to_string(shaderType));
                        sstream << pch << "\n";
                    } else {
                        sstream <<  res << "\n";
                    }
                } else {
                    sstream << pch << "\n";
                }

                pch = strtok (nullptr, "\n\r");
            }
            return sstream.str();
        }

        void logCurrentCompileException(GLuint &shader, GLenum type, vector<string> &errors) {
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
                    typeStr = std::string("Unknown error type: ") + std::to_string(type);
                    break;
            }
            LOG_ERROR("Shader compile error in %s: %s",typeStr.c_str() ,errorLog.data());
            errors.push_back(std::string(errorLog.data())+"##"+std::to_string(type));
        }

        bool linkProgram(GLuint mShaderProgram, std::vector<std::string>& errors){
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
                errors.push_back(errorLog.data());
                LOG_ERROR("Shader linker error: %s",errorLog.data());
                return false;
            }
            return true;
        }
    }

    const char *c_str(UniformType u) {
        switch (u){
            case UniformType::Int:
                return "int";
            case UniformType::Float:
                return "float";
            case UniformType::Mat3:
                return "mat3";
            case UniformType::Mat4:
                return "mat4";
            case UniformType::Vec3:
                return "vec3";
            case UniformType::Vec4:
                return "vec4";
            case UniformType::Texture:
                return "texture";
            case UniformType::TextureCube:
                return "textureCube";
            case UniformType::Invalid:
            default:
                return "invalid";
        }
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

    std::shared_ptr<Shader> Shader::ShaderBuilder::build() {
        std::vector<std::string> errors;
        return build(errors);
    }

    std::shared_ptr<Shader> Shader::ShaderBuilder::build(std::vector<std::string>& errors) {
        std::shared_ptr<Shader> shader;
        if (updateShader){
            shader = updateShader->shared_from_this();
        } else {
            if (name.length()==0){
                name = "Unnamed shader";
            }
            shader = std::shared_ptr<Shader>(new Shader());
        }
        bool compileSuccess = shader->build(shaderSources, errors);
        if (!compileSuccess){
            if (!updateShader) {
                shader.reset();
            }
            return shader;
        }
        shader->depthTest = this->depthTest;
        shader->depthWrite = this->depthWrite;
        shader->blend = this->blend;
        shader->name = this->name;
        shader->offset = this->offset;
        shader->shaderSources = this->shaderSources;
        return std::shared_ptr<Shader>(shader);
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withName(const std::string& name) {
        this->name = name;
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withOffset(float factor, float units) {
        this->offset = {factor, units};
        return *this;
    }

    Shader::ShaderBuilder & Shader::ShaderBuilder::withSourceString(const std::string &shaderSource, ShaderType shaderType) {
        this->shaderSources[shaderType] = {
            ResourceType::Memory,
            shaderSource
        };
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceFile(const std::string &shaderFile, ShaderType shaderType) {
        this->shaderSources[shaderType] = {
            ResourceType::File,
            shaderFile
        };
        return *this;
    }

    std::string Shader::translateToGLSLES(std::string source, bool vertexShader) {
        using namespace std;

        string replace = "#version 140";
        string replaceWith = vertexShader?"#version 100":"#version 100\n#ifdef GL_ES\n"
                "precision mediump float;\n"
                "#endif";
        size_t f = source.find(replace);
        source = source.replace(f, replace.length(), replaceWith);

        // replace textures
        if (vertexShader){
            regex regExpSearchShim3 { R"(\n\s*out\b)"};
            source = regex_replace(source, regExpSearchShim3, "\nvarying");
            regex regExpSearchShim4 { R"(\n\s*in\b)"};
            source = regex_replace(source, regExpSearchShim4, "\nattribute");
        } else {
            regex regExpSearchShim2 { R"(\bfragColor\b)"};
            source = regex_replace(source, regExpSearchShim2, "gl_FragColor");
            regex regExpSearchShim3 { R"(\n\s*out\b)"};
            source = regex_replace(source, regExpSearchShim3, "\n// out");
            regex regExpSearchShim4 { R"(\n\s*in\b)"};
            source = regex_replace(source, regExpSearchShim4, "\nvarying");
        }

        regex regExpSearchShim1 { R"(\s*uniform\s+sampler([\w]*)\s+([\w_]*)\s*;.*)"};
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
            regex regExpSearchShim4 { string{R"(texture\s*\(\s*)"}+val.first+"\\s*," };
            source = regex_replace(source, regExpSearchShim4, string{"texture"}+val.second+"("+val.first+",");
        }
        return source;
    }

    void Shader::updateUniformsAndAttributes() {
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
                case GL_FLOAT_VEC3:
                    uniformType = UniformType::Vec3;
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
                LOG_ERROR("Unsupported shader type %s name %s",type,name);
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
                u.arraySize = size;
                u.type = uniformType;
                uniforms.push_back(u);
            } else {
                if (strcmp(name, "g_model")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationModel = location;
                    } else {
                        LOG_ERROR("Invalid g_model uniform type. Expected mat4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_view")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationView = location;
                    } else {
                        LOG_ERROR("Invalid g_view uniform type. Expected mat4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_projection")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationProjection = location;
                    } else {
                        LOG_ERROR("Invalid g_projection uniform type. Expected mat4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_normal")==0){
                    if (uniformType == UniformType::Mat3){
                        uniformLocationNormal = location;
                    } else {
                        LOG_ERROR("Invalid g_normal uniform type. Expected mat3 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_viewport")==0){
                    if (uniformType == UniformType::Vec4){
                        uniformLocationViewport = location;
                    } else {
                        LOG_ERROR("Invalid g_normal uniform type. Expected vec4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_ambientLight")==0){
                    if (uniformType == UniformType::Vec3){
                        uniformLocationAmbientLight = location;
                    } else {
                        LOG_ERROR("Invalid g_ambientLight uniform type. Expected vec3 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_lightPosType")==0){
                    if (uniformType == UniformType::Vec4 && size == Renderer::maxSceneLights){
                        uniformLocationLightPosType = location;
                    } else {
                        LOG_ERROR("Invalid g_lightPosType uniform type. Expected vec4[Renderer::maxSceneLights] - was %s[%i].",c_str(uniformType),size);
                    }
                }
                if (strcmp(name, "g_lightColorRange")==0){
                    if (uniformType == UniformType::Vec4 && size == Renderer::maxSceneLights){
                        uniformLocationLightColorRange = location;
                    } else {
                        LOG_ERROR("Invalid g_lightPosType uniform type. Expected vec4[Renderer::maxSceneLights] - was %s[%i].",c_str(uniformType),size);
                    }
                }
            }
        }

        // update attributes
        attributes.clear();
        GLint attributeCount;
        glGetProgramiv(shaderProgramId, GL_ACTIVE_ATTRIBUTES, &attributeCount);

        for (int i=0;i<attributeCount;i++){
            const int nameSize = 50;
            GLchar name[nameSize];
            GLsizei nameLength;
            GLint size;
            GLenum type;
            glGetActiveAttrib(	shaderProgramId,
                                   i,
                                   nameSize,
                                   &nameLength,
                                   &size,
                                   &type,
                                   name);
            auto location = glGetAttribLocation( shaderProgramId, name);
            attributes[std::string(name)] = {location,type, size};
        }
    }

    Shader::Shader() {
        if (! Renderer::instance ){
            LOG_FATAL("Cannot instantiate sre::Shader before sre::Renderer is created.");
        }
        Renderer::instance->renderStats.shaderCount++;

        Renderer::instance->shaders.emplace_back(this);
    }

    Shader::~Shader() {
        auto r = Renderer::instance;
        if (r){
            r->renderStats.shaderCount--;

            r->shaders.erase(std::remove(r->shaders.begin(), r->shaders.end(), this));

            glDeleteShader(shaderProgramId);
        }
    }

    bool Shader::setLights(WorldLights* worldLights, glm::mat4 viewTransform){
        if (worldLights==nullptr){
            glUniform4f(uniformLocationAmbientLight, 0,0,0,0);
            static float noLight[Renderer::maxSceneLights*4];
            for (int i=0;i<Renderer::maxSceneLights*4;i++){
                noLight[i] = 0;
            }
            glUniform4fv(uniformLocationLightPosType, Renderer::maxSceneLights, noLight);
            glUniform4fv(uniformLocationLightColorRange, Renderer::maxSceneLights, noLight);
            return false;
        }
        if (uniformLocationAmbientLight != -1) {
            glUniform3fv(uniformLocationAmbientLight, 1, glm::value_ptr(worldLights->ambientLight));
        }
        if (uniformLocationLightPosType != -1 && uniformLocationLightColorRange != -1){
            glm::vec4 lightPosType[Renderer::maxSceneLights];
            glm::vec4 lightColorRange[Renderer::maxSceneLights];
            for (int i=0;i<Renderer::maxSceneLights;i++){
                auto light = worldLights->getLight(i);
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
                glUniform4fv(uniformLocationLightPosType, Renderer::maxSceneLights, glm::value_ptr(lightPosType[0]));
            }
            if (uniformLocationLightColorRange != -1) {
                glUniform4fv(uniformLocationLightColorRange, Renderer::maxSceneLights, glm::value_ptr(lightColorRange[0]));
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
                LOG_ERROR("Invalid blend value - was %i",(int)blend);
                break;
        }
        if (offset.x == 0 && offset.y==0){
            glDisable(GL_POLYGON_OFFSET_FILL);
#ifndef EMSCRIPTEN
            glDisable(GL_POLYGON_OFFSET_LINE);
            glDisable(GL_POLYGON_OFFSET_POINT);
#endif
        } else {
            glEnable(GL_POLYGON_OFFSET_FILL);
#ifndef EMSCRIPTEN
            glEnable(GL_POLYGON_OFFSET_LINE);
            glEnable(GL_POLYGON_OFFSET_POINT);
#endif
            glPolygonOffset(offset.x, offset.y);
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

    std::shared_ptr<Shader> Shader::getUnlit() {
        if (unlit != nullptr){
            return unlit;
        }

        unlit = create()
                .withSourceUnlit()
                .withName("Unlit")
                .build();
        return unlit;
    }

    std::shared_ptr<Shader> Shader::getUnlitSprite() {
        if (unlitSprite != nullptr){
            return unlitSprite;
        }

        unlitSprite =  create()
                .withSourceUnlitSprite()
                .withBlend(BlendType::AlphaBlending)
                .withDepthTest(false)
                .withName("Unlit Sprite")
                .build();
        return unlitSprite;
    }

    std::shared_ptr<Shader> Shader::getStandard() {
        if (standard != nullptr){
            return standard;
        }
        standard = create()
                .withSourceStandard()
                .withName("Standard")
                .build();
        return standard;
    }

    Uniform Shader::getUniformType(const std::string &name) {
		for (auto i = uniforms.cbegin(); i != uniforms.cend(); i++) {
			if (i->name.compare(name) == 0)
				return *i;
		}
		Uniform u;
		u.type = UniformType::Invalid;
		u.id = -1;
		u.arraySize = -1;
		return u;
    }

    std::shared_ptr<Shader> Shader::getStandardParticles() {
        if (standardParticles != nullptr){
            return standardParticles;
        }

        standardParticles = create()
                .withSourceStandardParticles()
                .withBlend(BlendType::AdditiveBlending)
                .withDepthWrite(false)
                .withName("Standard Particles")
                .build();
        return standardParticles;
    }

    Shader::ShaderBuilder Shader::create() {
        return Shader::ShaderBuilder();
    }

    bool Shader::build(std::map<ShaderType,Resource> shaderSources, std::vector<std::string>& errors) {
        unsigned int oldShaderProgramId = shaderProgramId;
        shaderProgramId = glCreateProgram();
        assert(shaderProgramId != 0);
        for (ShaderType i=ShaderType::Vertex;i<ShaderType::NumberOfShaderTypes;i = (ShaderType )((int)i+1)) {
            auto shaderSourcesIter = shaderSources.find(i);
            if (shaderSourcesIter!=shaderSources.end()){
                GLuint s;
                GLenum shader = to_id(i);

                bool res = compileShader(shaderSourcesIter->second, shader, s, errors);
                if (!res){
                    glDeleteProgram( shaderProgramId );
                    shaderProgramId = oldShaderProgramId;
                    return false;
                }
                glAttachShader(shaderProgramId,  s);
            }
        }

        bool linked = linkProgram(shaderProgramId, errors);
        if (!linked){
            glDeleteProgram( shaderProgramId );
            shaderProgramId = oldShaderProgramId;
            return false;
        }
        if (oldShaderProgramId != 0){
            glDeleteProgram( oldShaderProgramId );
        }
        updateUniformsAndAttributes();
        return true;
    }

    std::vector<std::string> Shader::getAttributeNames() {
        std::vector<std::string> res;
        for (auto& u : attributes){
            res.push_back(u.first);
        }
        return res;
    }

    std::vector<std::string> Shader::getUniformNames() {
        std::vector<std::string> res;
        for (auto& u : uniforms){
            res.push_back(u.name);
        }
        return res;
    }

    bool Shader::validateMesh(Mesh *mesh, std::string &info) {
        bool valid = true;
        for (auto& shaderVertexAttribute : attributes){
            auto meshType = mesh->getType(shaderVertexAttribute.first);
            if (meshType.first == -1){
                valid = false;
                info += "Cannot find vertex attribute '"+shaderVertexAttribute.first+"' in mesh of type ";
                if (shaderVertexAttribute.second.type == GL_FLOAT){
                    info += "float";
                } else if (shaderVertexAttribute.second.type == GL_FLOAT_VEC2){
                    info += "vec2";
                } else if (shaderVertexAttribute.second.type == GL_FLOAT_VEC3){
                    info += "vec3";
                } else if (shaderVertexAttribute.second.type == GL_FLOAT_VEC4){
                    info += "vec4";
                } else if (shaderVertexAttribute.second.type == GL_INT_VEC4){
                    info += "ivec4";
                }
                info += "\n";
            } else {

            }
        }
        return valid;
    }

    std::shared_ptr<Material> Shader::createMaterial(std::map<std::string,std::string> specializationConstants) {
        if (parent){
            return parent->createMaterial(std::move(specializationConstants));
        }
        if (specializationConstants.size()>0){
            for (auto & s : specializations){
                if (auto ptr = s.lock()){
                    if (map_compare(specializationConstants, ptr->specializationConstants)){
                        return std::shared_ptr<Material>(new Material(ptr));
                    }
                }
            }
            // no specialization shader found
            auto res =  Shader::ShaderBuilder();
            res.depthTest = this->depthTest;
            res.depthWrite = this->depthWrite;
            res.blend = this->blend;
            res.name = this->name;
            res.offset = this->offset;
            res.shaderSources = this->shaderSources;
            res.specializationConstants = specializationConstants;
            auto specializedShader = res.build();
            specializedShader->parent = shared_from_this();
            specializations.push_back(std::weak_ptr<Shader>(specializedShader));
            return std::shared_ptr<Material>(new Material(specializedShader));
        }
        return std::shared_ptr<Material>(new Material(shared_from_this()));
    }

    const std::string& Shader::getName() {
        return name;
    }

    std::string Shader::getSource(Shader::Resource &resource) {
        std::string source = resource.value;
        if (resource.resourceType==ResourceType::File){
            source = getFileContents(source);
        }
        return source;
    }

    bool Shader::compileShader(Resource& resource, GLenum type, GLuint& shader, std::vector<std::string>& errors){
        auto source = getSource(resource);
        std::string source_ = precompile(source, errors, type);
        shader = glCreateShader(type);
        auto stringPtr = source_.c_str();
        GLint length = (GLint)strlen(stringPtr);
        glShaderSource(shader, 1, &stringPtr, &length);
        glCompileShader(shader);
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (success == GL_FALSE){
            logCurrentCompileException(shader, type, errors);
            return false;
        }
        return true;
    }

    std::pair<int, int> Shader::getAttibuteType(const std::string &name) {
        auto res = attributes[name];
        return {res.type, res.arraySize};
    }

    glm::vec2 Shader::getOffset() {
        return offset;
    }

    std::string Shader::precompile(std::string source, std::vector<std::string>& errors, uint32_t shaderType) {


        // Replace includes with content
        // for each occurrence of #pragma include replace with substitute
        source = pragmaInclude(source, errors, shaderType);

        // Set engine defined shader constants
        source = std::regex_replace(source, std::regex("SCENE_LIGHTS"), std::to_string(Renderer::maxSceneLights));

#ifdef EMSCRIPTEN
        source = Shader::translateToGLSLES(source, type==GL_VERTEX_SHADER);
#endif
        return source;
    }

    Shader::ShaderBuilder Shader::update() {
        auto res =  Shader::ShaderBuilder(this);
        res.depthTest = this->depthTest;
        res.depthWrite = this->depthWrite;
        res.blend = this->blend;
        res.name = this->name;
        res.offset = this->offset;
        res.shaderSources = this->shaderSources;
        return res;
    }

    std::map<std::string,std::string> Shader::getCurrentSpecializationConstants() {
        return specializationConstants;
    }

    std::vector<std::string> Shader::getAllSpecializationConstants() {
        LOG_ERROR("getAllSpecializationConstants not implemented"); // todo fix
        return vector<string>();
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSource(const std::string& vertexShader, const std::string& fragmentShader) {
        withSourceString(vertexShader, ShaderType::Vertex);
        withSourceString(fragmentShader, ShaderType::Fragment);

        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceStandard() {
        withSourceFile("standard_vert.glsl", ShaderType::Vertex);
        withSourceFile("standard_frag.glsl", ShaderType::Fragment);
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceUnlit() {
        withSourceFile("unlit_vert.glsl", ShaderType::Vertex);
        withSourceFile("unlit_frag.glsl", ShaderType::Fragment);

        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceUnlitSprite() {
        withSourceFile("sprite_vert.glsl", ShaderType::Vertex);
        withSourceFile("sprite_frag.glsl", ShaderType::Fragment);

        return *this;
    }

    // The particle size used in this shader depends on the height of the screensize (to make the particles resolution independent):
    // for perspective projection, the size of particles are defined in screenspace size at the distance of 1.0 on a viewport of height 600.
    // for orthographic projection, the size of particles are defined in screenspace size on a viewport of height 600.
    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceStandardParticles() {
        withSourceFile("particles_vert.glsl", ShaderType::Vertex);
        withSourceFile("particles_frag.glsl", ShaderType::Fragment);

        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceDebugUV() {
        withSourceFile("debug_uv_vert.glsl", ShaderType::Vertex);
        withSourceFile("debug_uv_frag.glsl", ShaderType::Fragment);

        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withSourceDebugNormals() {
        withSourceFile("debug_normal_vert.glsl", ShaderType::Vertex);
        withSourceFile("debug_normal_frag.glsl", ShaderType::Fragment);

        return *this;
    }

    Shader::ShaderBuilder::ShaderBuilder(Shader *shader)
    :updateShader(shader)
    {
    }

    uint32_t to_id(ShaderType st) {
        uint32_t shader;
        switch (st){
            case ShaderType::Vertex:
                shader = GL_VERTEX_SHADER;
                break;
            case ShaderType::Fragment:
                shader = GL_FRAGMENT_SHADER;
                break;
#ifndef EMSCRIPTEN
            case ShaderType::Geometry:
                shader = GL_GEOMETRY_SHADER;
                break;
            case ShaderType::TessellationEvaluation:
                shader = GL_TESS_EVALUATION_SHADER ;
                break;
            case ShaderType::TessellationControl:
                shader = GL_TESS_CONTROL_SHADER;
                break;
#endif
            default:
                LOG_ERROR("Invalid shader type. Was %d",(int)st);
                break;
        }
        return shader;
    }
}
