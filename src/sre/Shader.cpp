/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/Shader.hpp"
#include "sre/Material.hpp"
#include "sre/impl/ShaderSource.inl"

#include <fstream>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <regex>
#include <sre/Log.hpp>
#include "sre/Renderer.hpp"


using namespace std;

namespace sre {
    // anonymous (file local) namespace
    namespace {
        std::shared_ptr<Shader> standardPBR;
        std::shared_ptr<Shader> standardBlinnPhong;
        std::shared_ptr<Shader> standardPhong;
        std::shared_ptr<Shader> unlit;
        std::shared_ptr<Shader> skybox;
        std::shared_ptr<Shader> skyboxProcedural;
        std::shared_ptr<Shader> blit;
        std::shared_ptr<Shader> unlitSprite;
        std::shared_ptr<Shader> standardParticles;

        long globalShaderCounter = 1;

        // From https://stackoverflow.com/a/8473603/420250
        template <typename Map>
        bool map_compare (Map const &lhs, Map const &rhs) {
            // No predicate needed because there is operator== for pairs already.
            return lhs.size() == rhs.size()
                   && std::equal(lhs.begin(), lhs.end(),
                                 rhs.begin());
        }

        template<typename Out>
        void split(const std::string &s, char delim, Out result) {
            std::stringstream ss(s);
            std::string item;
            while (std::getline(ss, item, delim)) {
                *(result++) = item;
            }
        }

        std::vector<std::string> split(const std::string &s, char delim) {
            std::vector<std::string> elems;
            split(s, delim, std::back_inserter(elems));
            return elems;
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

            std::regex e ( R"_(#pragma\s+include\s+"([^"]*)")_", std::regex::ECMAScript);
            int lineNumber = 0;
            std::vector<std::string> lines = split(source, '\n');
            int includes = 0;
            for (auto& s : lines)
            {
                lineNumber++;

                std::smatch m;
                if (std::regex_search (s,m,e)) {
                    std::string match = m[1];
                    auto res = getFileContents(match);
                    if (res.empty()){
                        errors.push_back(std::string("0:")+std::to_string(lineNumber)+" cannot find include file "+match+"##"+std::to_string(shaderType));
                        sstream << s << "\n";
                    } else {
                        includes++;
                        sstream << "#line "<<(includes*10000+1) <<"\n";
                        sstream << res << "\n";
                        sstream << "#line "<<lineNumber << "\n";
                    }
                } else {
                    sstream << s << "\n";
                }
            }
            return sstream.str();
        }

        void logCurrentCompileInfo(GLuint &shader, GLenum type, vector<string> &errors, const std::string& source, const std::string name) {
            GLint logSize = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
            if (logSize > 1){ // log size of 1 is empty, since it includes \0
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

                LOG_ERROR("Shader compile error in %s (%s): %s", name.c_str(), typeStr.c_str(), errorLog.data());
                errors.push_back(std::string(errorLog.data())+"##"+std::to_string(type));

            }
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
                if (logSize > 1){ // log size of 1 is empty, since it includes \0
                    std::vector<char> errorLog((size_t) logSize);
                    glGetProgramInfoLog(mShaderProgram, logSize, nullptr, errorLog.data() );
                    errors.emplace_back(errorLog.data());
                    LOG_ERROR("Shader linker error: %s",errorLog.data());
                }
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


    Shader::ShaderBuilder &Shader::ShaderBuilder::withColorWrite(glm::bvec4 enable) {
        this->colorWrite = enable;
        return *this;
    }

    Shader::ShaderBuilder &Shader::ShaderBuilder::withStencil(Stencil stencil) {
        this->stencil = std::move(stencil);
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
            shader->specializationConstants = this->specializationConstants;
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
        shader->shaderUniqueId = globalShaderCounter++;
        shader->stencil = stencil;
        shader->colorWrite = colorWrite;
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

    std::string Shader::translateToGLSLES(std::string source, bool vertexShader, int version) {
        using namespace std;
        string replace = "#version 330";
        string replaceWith = string("#version ")+std::to_string(version);
        if (version >100){
            replaceWith += " es";
        }

        size_t f = source.find(replace);
        if (f != std::string::npos){
            source = source.replace(f, replace.length(), replaceWith);
        }
        if (!vertexShader){
            auto extension = source.rfind("#extension");
            // insert precision after extensions
            long insertPrecisionPos = string::npos;
            if (extension != string::npos){
                insertPrecisionPos = source.find('\n',extension);
            } else {
                insertPrecisionPos = source.find('\n');
            }
            if (insertPrecisionPos == string::npos){
                LOG_ERROR("insertPrecisionPos invalid");
                insertPrecisionPos = 0;
            }
            source = source.substr(0, insertPrecisionPos)+
                     "\n"
                     "#ifdef GL_FRAGMENT_PRECISION_HIGH \n"
                     "   precision highp float;         \n"
                     "#else                             \n"
                     "   precision mediump float;       \n"
                     "#endif                            \n"
                     "#line 2"+
                     source.substr(insertPrecisionPos);
        }
        if (version == 100){
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
                if (!match.empty()){
                    string samplerType = match[1].str();
                    string samplerName = match[2].str();
                    textureType[samplerName] = samplerType;
                }
            }

            for (auto val : textureType){
                regex regExpSearchShim4 { string{R"(texture\s*\(\s*)"}+val.first+"\\s*," };
                source = regex_replace(source, regExpSearchShim4, string{"texture"}+val.second+"("+val.first+",");
            }
        }
        return source;
    }

    void Shader::updateUniformsAndAttributes() {
        uniformLocationModel = -1;
        uniformLocationView = -1;
        uniformLocationProjection = -1;
        uniformLocationModelViewInverseTranspose = -1;
		uniformLocationModelInverseTranspose = -1;
        uniformLocationViewport = -1;
        uniformLocationAmbientLight = -1;
        uniformLocationLightPosType = -1;
        uniformLocationLightColorRange = -1;
        uniformLocationCameraPosition = -1;
        uniforms.clear();

        bool hasGlobalUniformBuffer = false;
        if (Renderer::instance->globalUniformBuffer) {
            hasGlobalUniformBuffer = glGetUniformBlockIndex(shaderProgramId, "g_global_uniforms") != GL_INVALID_INDEX;
        }

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
                case GL_INT_VEC4:
                    uniformType = UniformType::IVec4;
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
                if (Renderer::instance->globalUniformBuffer){
                    if (strncmp(name, "g_model_it",64)!=0 &&
                        strncmp(name, "g_model_view_it",64)!=0 &&
                        strncmp(name, "g_model",64)!=0){
                        if (!hasGlobalUniformBuffer){
                            // Check using old style non uniform buffer
                            LOG_ERROR("global uniform %s must be loaded using #pragma include \"global_uniforms_incl.glsl\"", name);
                        }
                        continue;
                    }
                }
                if (strcmp(name, "g_model")==0){
                    if (uniformType == UniformType::Mat4){
                        uniformLocationModel = location;
                    } else {
                        LOG_ERROR("Invalid g_model uniform type. Expected mat4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_model_it")==0){
                    if (uniformType == UniformType::Mat3){
                        uniformLocationModelInverseTranspose = location;
                    } else {
                        LOG_ERROR("Invalid g_model_it uniform type. Expected mat3 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_model_view_it")==0){
                    if (uniformType == UniformType::Mat3){
                        uniformLocationModelViewInverseTranspose = location;
                    } else {
                        LOG_ERROR("Invalid g_model_view_it uniform type. Expected mat3 - was %s.",c_str(uniformType));
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
                if (strcmp(name, "g_viewport")==0){
                    if (uniformType == UniformType::Vec4){
                        uniformLocationViewport = location;
                    } else {
                        LOG_ERROR("Invalid g_viewport uniform type. Expected vec4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_ambientLight")==0){
                    if (uniformType == UniformType::Vec4){
                        uniformLocationAmbientLight = location;
                    } else {
                        LOG_ERROR("Invalid g_ambientLight uniform type. Expected vec4 - was %s.",c_str(uniformType));
                    }
                }
                if (strcmp(name, "g_lightPosType")==0){
                    if (uniformType == UniformType::Vec4 && size == Renderer::instance->maxSceneLights){
                        uniformLocationLightPosType = location;
                    } else {
                        LOG_ERROR("Invalid g_lightPosType uniform type. Expected vec4[Renderer::maxSceneLights] - was %s[%i].",c_str(uniformType),size);
                    }
                }
                if (strcmp(name, "g_lightColorRange")==0){
                    if (uniformType == UniformType::Vec4 && size == Renderer::instance->maxSceneLights){
                        uniformLocationLightColorRange = location;
                    } else {
                        LOG_ERROR("Invalid g_lightPosType uniform type. Expected vec4[Renderer::maxSceneLights] - was %s[%i].",c_str(uniformType),size);
                    }
                }
                if (strcmp(name, "g_cameraPos")==0){
                    if (uniformType == UniformType::Vec4){
                        uniformLocationCameraPosition = location;
                    } else {
                        LOG_ERROR("Invalid g_cameraPos uniform type. Expected vec4 - was %s[%i].",c_str(uniformType),size);
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

    bool Shader::setLights(WorldLights* worldLights){
        int maxSceneLights = Renderer::instance->maxSceneLights;
        if (worldLights == nullptr){
            glUniform4f(uniformLocationAmbientLight, 0,0,0,0);
            const int vec4Elements = 4;
			std::vector<float> noLight(maxSceneLights * vec4Elements, 0.0f);
            glUniform4fv(uniformLocationLightPosType, maxSceneLights, noLight.data());
            glUniform4fv(uniformLocationLightColorRange, maxSceneLights, noLight.data());
            return false;
        }
        if (uniformLocationAmbientLight != -1) {
            glUniform4fv(uniformLocationAmbientLight, 1, glm::value_ptr(worldLights->ambientLight));
        }
        if (uniformLocationLightPosType != -1 && uniformLocationLightColorRange != -1){
			std::vector<glm::vec4> lightPosType(maxSceneLights, glm::vec4(0));
			std::vector<glm::vec4> lightColorRange(maxSceneLights, glm::vec4(0));
            for (int i=0;i<maxSceneLights;i++){
                auto light = worldLights->getLight(i);
                if (light == nullptr || light->lightType == LightType::Unused) {
                    lightPosType[i] = glm::vec4(0.0f,0.0f,0.0f, 2);
                    continue;
                } else if (light->lightType == LightType::Point) {
                    lightPosType[i] = glm::vec4(light->position, 1);
                } else if (light->lightType == LightType::Directional) {
                    lightPosType[i] = glm::vec4(glm::normalize(light->direction), 0);
                }
                // transform to eye space
                lightColorRange[i] = glm::vec4(light->color, light->range);

            }
            if (uniformLocationLightPosType != -1) {
                glUniform4fv(uniformLocationLightPosType, maxSceneLights, glm::value_ptr(lightPosType[0]));
            }
            if (uniformLocationLightColorRange != -1) {
                glUniform4fv(uniformLocationLightColorRange, maxSceneLights, glm::value_ptr(lightColorRange[0]));
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
        if (stencil.func == StencilFunc::Disabled){
            glDisable(GL_STENCIL_TEST);
            glStencilMask(0);
        } else {
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(static_cast<GLenum>(stencil.func), (GLint)stencil.ref, (GLint)stencil.mask);
            glStencilOp(static_cast<GLenum>(stencil.fail),static_cast<GLenum>(stencil.zfail),static_cast<GLenum>(stencil.zpass));
            glStencilMask(0xFFFF);
        }
        GLboolean dm = (GLboolean) (depthWrite ? GL_TRUE : GL_FALSE);
        glDepthMask(dm);
        glColorMask(colorWrite.r, colorWrite.g, colorWrite.b, colorWrite.a);
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
        auto& info = renderInfo();
        if (offset.x == 0 && offset.y==0){
            glDisable(GL_POLYGON_OFFSET_FILL);
#ifndef GL_ES_VERSION_2_0
            // GL_POLYGON_OFFSET_LINE and GL_POLYGON_OFFSET_POINT nor defined in ES 2.x or ES 3.x
            glDisable(GL_POLYGON_OFFSET_LINE);
            glDisable(GL_POLYGON_OFFSET_POINT);
#endif
        } else {
            glEnable(GL_POLYGON_OFFSET_FILL);
#ifndef GL_ES_VERSION_2_0
            // GL_POLYGON_OFFSET_LINE and GL_POLYGON_OFFSET_POINT nor defined in ES 2.x or ES 3.x
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
                .withSourceFile("unlit_vert.glsl", ShaderType::Vertex)
                .withSourceFile("unlit_frag.glsl", ShaderType::Fragment)
                .withName("Unlit")
                .build();
        return unlit;
    }



    std::shared_ptr<Shader> Shader::getSkybox() {
        if (skybox != nullptr){
            return skybox;
        }

        skybox = create()
                .withSourceFile("skybox_vert.glsl", ShaderType::Vertex)
                .withSourceFile("skybox_frag.glsl", ShaderType::Fragment)
                .withName("Skybox")
                .withDepthWrite(false)
                .build();
        return skybox;
    }


    std::shared_ptr<Shader> Shader::getSkyboxProcedural() {
        if (skyboxProcedural != nullptr){
            return skyboxProcedural;
        }

        skyboxProcedural = create()
                .withSourceFile("skybox_proc_vert.glsl", ShaderType::Vertex)
                .withSourceFile("skybox_proc_frag.glsl", ShaderType::Fragment)
                .withName("Skybox Procedural")
                .withDepthWrite(false)
                .build();

        return skyboxProcedural;
    }


    std::shared_ptr<Shader> Shader::getBlit() {
        if (blit != nullptr){
            return blit;
        }

        blit = create()
                .withSourceFile("blit_vert.glsl", ShaderType::Vertex)
                .withSourceFile("blit_frag.glsl", ShaderType::Fragment)
                .withName("Blit")
                .build();
        return blit;
    }

    std::shared_ptr<Shader> Shader::getUnlitSprite() {
        if (unlitSprite != nullptr){
            return unlitSprite;
        }

        unlitSprite =  create()
                .withSourceFile("sprite_vert.glsl", ShaderType::Vertex)
                .withSourceFile("sprite_frag.glsl", ShaderType::Fragment)
                .withBlend(BlendType::AlphaBlending)
                .withDepthTest(false)
                .withName("Unlit Sprite")
                .build();
        return unlitSprite;
    }

    std::shared_ptr<Shader> Shader::getStandard(){
        return getStandardBlinnPhong();
    }

    std::shared_ptr<Shader> Shader::getStandardPBR(){
        if (standardPBR != nullptr){
            return standardPBR;
        }
        standardPBR = create()
                .withSourceFile("standard_pbr_vert.glsl", ShaderType::Vertex)
                .withSourceFile("standard_pbr_frag.glsl", ShaderType::Fragment)
                .withName("Standard")
                .build();
        return standardPBR;
    }

    Uniform Shader::getUniform(const std::string &name) {
		for (auto& uniform : uniforms) {
			if (uniform.name == name)
				return uniform;
		}
		Uniform u;
		u.type = UniformType::Invalid;
		u.id = -1;
		u.arraySize = -1;
		return u;
    }

    // The particle size used in this shader depends on the height of the screensize (to make the particles resolution independent):
    // for perspective projection, the size of particles are defined in screenspace size at the distance of 1.0 on a viewport of height 600.
    // for orthographic projection, the size of particles are defined in screenspace size on a viewport of height 600.
    std::shared_ptr<Shader> Shader::getStandardParticles() {
        if (standardParticles != nullptr){
            return standardParticles;
        }

        standardParticles = create()
                .withSourceFile("particles_vert.glsl", ShaderType::Vertex)
                .withSourceFile("particles_frag.glsl", ShaderType::Fragment)
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
        std::vector<GLuint> shaders;

        auto cleanupShaders = [&](){
            for (auto id : shaders){
                glDeleteShader(id);
            }
        };

        for (ShaderType i=ShaderType::Vertex;i<ShaderType::NumberOfShaderTypes;i = (ShaderType )((int)i+1)) {
            auto shaderSourcesIter = shaderSources.find(i);
            if (shaderSourcesIter!=shaderSources.end()) {
                GLuint s;
                GLenum shader = to_id(i);
                bool res = compileShader(shaderSourcesIter->second, shader, s, errors);
                if (!res) {
                    cleanupShaders();
                    glDeleteProgram( shaderProgramId );
                    shaderProgramId = oldShaderProgramId;
                    return false;
                } else {
                    shaders.push_back(s);
                }
                glAttachShader(shaderProgramId,  s);
            }
        }

        bool linked = linkProgram(shaderProgramId, errors);
        cleanupShaders();
        if (!linked) {
            glDeleteProgram( shaderProgramId );
            shaderProgramId = oldShaderProgramId; // revert to old shader
            return false;
        }
        if (oldShaderProgramId != 0){
            glDeleteProgram( oldShaderProgramId ); // delete old shader if any
        }
        // setup global uniform
        if (Renderer::instance->globalUniformBuffer){
            glUseProgram(shaderProgramId);
            auto index = glGetUniformBlockIndex(shaderProgramId, "g_global_uniforms");
            if (index != GL_INVALID_INDEX){
                const int globalUniformBindingIndex = 1;
                glUniformBlockBinding(shaderProgramId, index, globalUniformBindingIndex);
                glBindBufferRange(GL_UNIFORM_BUFFER, globalUniformBindingIndex,
                                  Renderer::instance->globalUniformBuffer, 0, Renderer::instance->globalUniformBufferSize);
            }
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
        if (!specializationConstants.empty()){
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
            if (specializedShader == nullptr){
                LOG_WARNING("Cannot create specialized shader. Using shader without specialization.");
                return std::shared_ptr<Material>(new Material(shared_from_this()));
            }
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
        auto length = (GLint)strlen(stringPtr);
        glShaderSource(shader, 1, &stringPtr, &length);
        glCompileShader(shader);
        GLint success = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        logCurrentCompileInfo(shader, type, errors, source_, resource.value);

        return success == 1;
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

        // Insert preprocessor define symbols
        source = insertPreprocessorDefines(source, specializationConstants, shaderType);

        if (renderInfo().graphicsAPIVersionES) {
            source = Shader::translateToGLSLES(source, shaderType == GL_VERTEX_SHADER, renderInfo().graphicsAPIVersionMajor<=2?100:300);
        }
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

    std::set<std::string> Shader::getAllSpecializationConstants() {
        if (parent){
            return parent->getAllSpecializationConstants();
        }
        static std::regex SPECIALIZATION_CONSTANT_PATTERN("(S_[A-Z_0-9]+)");
        std::set<string> res;
        for (auto& source : shaderSources){
            string s = getSource(source.second);
            std::smatch m;
            while (std::regex_search(s, m, SPECIALIZATION_CONSTANT_PATTERN)) {
                std::string match = m.str();
                res.insert(match);
                s = m.suffix();
            }
        }
        return res;
    }

    std::shared_ptr<Shader> Shader::getStandardBlinnPhong() {
        if (standardBlinnPhong != nullptr){
            return standardBlinnPhong;
        }
        standardBlinnPhong = create()
                .withSourceFile("standard_blinn_phong_vert.glsl", ShaderType::Vertex)
                .withSourceFile("standard_blinn_phong_frag.glsl", ShaderType::Fragment)
                .withName("StandardBlinnPhong")
                .build();
        return standardBlinnPhong;
    }
    std::shared_ptr<Shader> Shader::getStandardPhong() {
        if (standardPhong != nullptr){
            return standardPhong;
        }
        standardPhong = create()
                .withSourceFile("standard_phong_vert.glsl", ShaderType::Vertex)
                .withSourceFile("standard_phong_frag.glsl", ShaderType::Fragment)
                .withName("StandardPhong")
                .build();
        return standardPhong;
    }


    Shader::ShaderBuilder &Shader::ShaderBuilder::withSource(const std::string& vertexShader, const std::string& fragmentShader) {
        withSourceString(vertexShader, ShaderType::Vertex);
        withSourceString(fragmentShader, ShaderType::Fragment);

        return *this;
    }

    Shader::ShaderBuilder::ShaderBuilder(Shader *shader)
    :updateShader(shader)
    {
    }

    std::string Shader::insertPreprocessorDefines(std::string source,
                                          std::map<std::string, std::string> &specializationConstants,
                                          uint32_t shaderType){
        stringstream ss;

        ss<<"#define SI_LIGHTS "<<Renderer::instance->maxSceneLights<<"\n";
        // add shader type
        switch (shaderType){
            case GL_FRAGMENT_SHADER:
                ss<<"#define SI_FRAGMENT 1\n";
                break;
            case GL_VERTEX_SHADER:
                ss<<"#define SI_VERTEX 1\n";
                break;
#ifndef EMSCRIPTEN
            case GL_GEOMETRY_SHADER:
                ss<<"#define SI_GEOMETRY 1\n";
                break;
            case GL_TESS_CONTROL_SHADER:
                ss<<"#define SI_TESS_CTRL 1\n";
                break;
            case GL_TESS_EVALUATION_SHADER:
                ss<<"#define SI_TESS_EVAL 1\n";
                break;
#endif
            default:
                LOG_WARNING("Unknown shader type");
                break;
        }
        for (auto & sc : specializationConstants){
            ss<<"#define "<<sc.first<<" "<<sc.second<<"\n";
        }

        if (renderInfo().useFramebufferSRGB){
            ss<<"#define SI_FRAMEBUFFER_SRGB 1\n";
        }
        if (renderInfo().supportTextureSamplerSRGB){
            ss<<"#define SI_TEX_SAMPLER_SRGB 1\n";
        }

        // If the shader contains any #version or #extension statements, the defines are added after them.
        auto version = static_cast<int>(source.rfind("#version"));
        auto extension = static_cast<int>(source.rfind("#extension"));
        auto last = std::max(version, extension);
        if (last == -1){
            ss << "#line 1\n";
            return ss.str()+source;
        }
        auto insertPos = source.find('\n', last);
        int lines = 0;
        for (int i=0;i<insertPos;i++){
            if (source.at(i) == '\n'){
                lines++;
            }
        }
        ss << "#line "<<(lines+1)<<"\n";
        return source.substr(0, insertPos+1) +
               ss.str()+
               source.substr(insertPos+1);
    }

    Stencil Shader::getStencil() {
        return stencil;
    }

    glm::bvec4 Shader::getColorWrite() {
        return colorWrite;
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
                shader = GL_VERTEX_SHADER;
                LOG_ERROR("Invalid shader type. Was %d",(int)st);
                break;
        }
        return shader;
    }
}
