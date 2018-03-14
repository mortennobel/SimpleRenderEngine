/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */
#include "sre/Inspector.hpp"
#include "TextEditor.h"
#include <algorithm>
#include <iostream>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include "sre/Renderer.hpp"
#include "sre/SDLRenderer.hpp"
#include "sre/impl/GL.hpp"
#include "sre/Texture.hpp"
#include "sre/SpriteAtlas.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/Sprite.hpp"
#include "imgui_internal.h"
#include <SDL_image.h>

using Clock = std::chrono::high_resolution_clock;
using Milliseconds = std::chrono::duration<float, std::chrono::milliseconds::period>;

namespace sre {
    namespace {
        std::string appendSize(std::string s, int size) {
            if (size>1){
                s += "["+std::to_string(size)+"]";
            }
            return s;
        }

        std::string glEnumToString(int type) {
            std::string typeStr = "unknown";
            switch (type){
                case GL_FLOAT:
                    typeStr = "float";
                    break;
                case GL_INT:
                    typeStr = "int";
                    break;
                case GL_FLOAT_VEC2:
                    typeStr = "vec2";
                    break;
                case GL_FLOAT_VEC3:
                    typeStr = "vec3";
                    break;
                case GL_FLOAT_VEC4:
                    typeStr = "vec4";
                    break;
                case GL_FLOAT_MAT4:
                    typeStr = "mat4";
                    break;
                case GL_FLOAT_MAT3:
                    typeStr = "mat3";
                    break;
                case GL_INT_VEC4:
                    typeStr = "ivec4";
                    break;
                default:
                    typeStr = "invalid";
            }
            return typeStr;
        }
    }

    Inspector::Inspector(int frames)
            :frames(frames),frameCount(0)
    {
        stats.resize(frames);
        millisecondsFrameTime.resize(frames);
        if (SDLRenderer::instance){
            millisecondsEvent.resize(frames);
            millisecondsUpdate.resize(frames);
            millisecondsRender.resize(frames);
        }

        data.resize(frames);
        lastTick = Clock::now();
    }

    void Inspector::showTexture(Texture* tex){
        std::string s = tex->getName()+"##"+std::to_string((int64_t)tex);
        if (ImGui::TreeNode(s.c_str())){

            ImGui::LabelText("Size","%ix%i",tex->getWidth(),tex->getHeight());
            ImGui::LabelText("Cubemap","%s",tex->isCubemap()?"true":"false");
            const char* depthStr;
            switch (tex->getDepthPrecision()){
                case Texture::DepthPrecision::I16:                // 16 bit integer
                    depthStr = "16 bit";
                    break;
                case Texture::DepthPrecision::I24:                // 24 bit integer
                    depthStr = "24 bit";
                    break;
                case Texture::DepthPrecision::I32:                // 32 bit integer
                    depthStr = "32 bit";
                    break;
                case Texture::DepthPrecision::F32:                // 32 bit float
                    depthStr = "32 bit float";
                    break;
                case Texture::DepthPrecision::I24_STENCIL8:       // 24 bit integer 8 bit stencil
                    depthStr = "24 bit + 8 bit stencil";
                    break;
                case Texture::DepthPrecision::F32_STENCIL8:       // 32 bit float 8 bit stencil
                    depthStr = "32 bit float + 8 bit stencil";
                    break;
                case Texture::DepthPrecision::STENCIL8:           // 8 bit stencil
                    depthStr = "8 bit stencil";
                    break;
                case Texture::DepthPrecision::None:
                    depthStr = "None";
                    break;
            }
            ImGui::LabelText("Depth",depthStr);
            ImGui::LabelText("Filtersampling","%s",tex->isFilterSampling()?"true":"false");
            ImGui::LabelText("Mipmapping","%s",tex->isMipmapped()?"true":"false");
            ImGui::LabelText("Transparent","%s",tex->isTransparent()?"true":"false");
            const char* colorSpace;
            if (tex->getSamplerColorSpace() == Texture::SamplerColorspace::Gamma){
                colorSpace = "Gamma";
            } else {
                colorSpace = "Linear";
            }
            ImGui::LabelText("Colorspace", "%s", colorSpace);
            ImGui::LabelText("Wrap tex-coords","%s",tex->isWrapTextureCoordinates()?"true":"false");
            ImGui::LabelText("Data size","%f MB",tex->getDataSize()/(1000*1000.0f));
            if (!tex->isCubemap()){
                ImGui::Image(reinterpret_cast<ImTextureID>(tex->textureId), ImVec2(previewSize, previewSize),{0,1},{1,0},{1,1,1,1},{0,0,0,1});
            }

            ImGui::TreePop();
        }
    }

    void Inspector::showMesh(Mesh* mesh){
        std::string s = mesh->getName()+"##"+std::to_string((int64_t)mesh);
        if (ImGui::TreeNode(s.c_str())){
            ImGui::LabelText("Vertex count", "%i", mesh->getVertexCount());
            ImGui::LabelText("Mesh size", "%.2f MB", mesh->getDataSize()/(1000*1000.0f));
            if (ImGui::TreeNode("Vertex attributes")){
                auto attributeNames = mesh->getAttributeNames();
                for (auto & a : attributeNames) {
                    auto type = mesh->getType(a);
                    std::string typeStr = glEnumToString(type.first);
                    typeStr = appendSize(typeStr, type.second);
                    ImGui::LabelText(a.c_str(), "%s (%i)",typeStr.c_str(),type.first);
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Index sets")) {
                if (mesh->getIndexSets()==0){
                    ImGui::LabelText("", "None");
                } else {
                    for (int i=0;i<mesh->getIndexSets();i++){
                        char res[128];
                        sprintf(res,"Index %i size",i);
                        ImGui::LabelText(res, "%i", mesh->getIndicesSize(i));
                    }
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Mesh Data")) {
                auto interleavedData = mesh->getInterleavedData();
                auto attributes = mesh->attributeByName;
                for (auto& att : attributes){
                    auto attributeName = att.first;
                    if (ImGui::TreeNode(attributeName.c_str())) {
                        auto attributeType = att.second.attributeType;
                        auto attributeTypeStr = std::to_string(attributeType);
                        ImGui::LabelText("attributeType", attributeTypeStr.c_str());
                        auto dataType = att.second.dataType;
                        auto dataTypeStr = std::to_string(dataType);
                        ImGui::LabelText("dataType", dataTypeStr.c_str());
                        auto elementCount = att.second.elementCount;
                        auto elementCountStr = std::to_string(elementCount);
                        ImGui::LabelText("elementCount", elementCountStr.c_str());
                        auto offset = att.second.offset;
                        auto offsetStr = std::to_string(offset);
                        ImGui::LabelText("offset", offsetStr.c_str());
                        if (dataType == GL_INT){
                            for (int j=0;j<std::min(5,mesh->vertexCount); j++){
                                std::string value = "";
                                for (int i=0;i<att.second.elementCount;i++){
                                    float* data = &interleavedData[att.second.offset/sizeof(float)+i + (j*mesh->totalBytesPerVertex)/sizeof(float)];
                                    int* dataInt = reinterpret_cast<int*>(data);
                                    value += std::to_string(*dataInt)+" ";
                                }
                                std::string label = "Value ";
                                label+= std::to_string(j);
                                ImGui::LabelText(label.c_str(), value.c_str());
                            }
                        } else {
                            for (int j=0;j<std::min(5,mesh->vertexCount); j++){
                                std::string value = "";
                                for (int i=0;i<att.second.elementCount;i++){
                                    float data = interleavedData[att.second.offset/sizeof(float)+i + (j*mesh->totalBytesPerVertex)/sizeof(float)];
                                    value += std::to_string(data)+" ";
                                }
                                std::string label = "Value ";
                                label+= std::to_string(j);
                                ImGui::LabelText(label.c_str(), value.c_str());
                            }
                        }

                        ImGui::TreePop();
                    }
                    ;
                    auto attributeType = att.second.attributeType;

                }
                ImGui::TreePop();

            }
            if (mesh->hasAttribute("position")){
                initFramebuffer();

                Camera camera;
                camera.setPerspectiveProjection(60,0.1,10);
                camera.lookAt({0,0,4},{0,0,0},{0,1,0});
                auto offscreenTexture = getTmpTexture();
                framebuffer->setColorTexture(offscreenTexture);

                auto renderToTexturePass = RenderPass::create()
                     .withCamera(camera)
                     .withWorldLights(&worldLights)
                     .withFramebuffer(framebuffer)
                     .withClearColor(true, {0, 0, 0, 1})
                     .withGUI(false)
                     .build();
                static auto litMat = Shader::getStandardBlinnPhong()->createMaterial();
                static auto unlitMat = Shader::getUnlit()->createMaterial();

                bool hasNormals = mesh->getNormals().size()>0;
                auto mat = hasNormals ? litMat : unlitMat;
                auto sharedPtrMesh = mesh->shared_from_this();
                float rotationSpeed = 0.001f;

                auto bounds = mesh->getBoundsMinMax();
                auto center = (bounds[1] + bounds[0])*0.5f;
                auto offset = -center;
                auto scale = bounds[1]-bounds[0];
                float maxS = std::max({scale.x,scale.y,scale.z});

                std::vector<std::shared_ptr<Material>> mats;
                for (int m = 0;m<std::max(1,sharedPtrMesh->getIndexSets());m++){
                    mats.push_back(mat);
                }
                renderToTexturePass.draw(sharedPtrMesh, glm::eulerAngleY(time*rotationSpeed)*glm::scale(glm::vec3{2.0f/maxS,2.0f/maxS,2.0f/maxS})*glm::translate(offset), mats);

                ImGui::Image(reinterpret_cast<ImTextureID>(offscreenTexture->textureId), ImVec2(previewSize, previewSize),{0,1},{1,0},{1,1,1,1},{0,0,0,1});
            } else {
                ImGui::LabelText("", "No preview - missing position attribute");
            }
            ImGui::TreePop();
        }
    }

    std::string glUniformToString(UniformType type);

    void Inspector::showShader(Shader* shader){
        auto specialization = shader->getCurrentSpecializationConstants();
        std::string s = shader->getName()+(specialization.empty()?"":" Specialized")+"##"+std::to_string((int64_t)shader);
        if (ImGui::TreeNode(s.c_str())){
            if (ImGui::Button("Edit")) {
                shaderEdit = std::weak_ptr<Shader>(shader->shared_from_this());
            }
            if (ImGui::TreeNode("Attributes")) {
                auto attributeNames = shader->getAttributeNames();
                for (auto a : attributeNames){
                    auto type = shader->getAttibuteType(a);
                    std::string typeStr = glEnumToString(type.first);
                    typeStr = appendSize(typeStr, type.second);
                    ImGui::LabelText(a.c_str(), typeStr.c_str());
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Uniforms")) {
                auto uniformNames = shader->getUniformNames();
                for (auto a : uniformNames){
                    auto type = shader->getUniformType(a);
                    std::string typeStr = glUniformToString(type.type);
                    typeStr = appendSize(typeStr, type.arraySize);
                    ImGui::LabelText(a.c_str(), typeStr.c_str());
                }
                ImGui::TreePop();
            }
            if (ImGui::TreeNode("Specialization")) {
                for (auto a : specialization ){
                    ImGui::LabelText(a.first.c_str(), a.second.c_str());
                }
                ImGui::TreePop();
            }

            auto blend = shader->getBlend();
            std::string s;
            switch (blend){
                case BlendType::AdditiveBlending:
                    s = "Additive blending";
                    break;
                case BlendType::AlphaBlending:
                    s = "Alpha blending";
                    break;
                case BlendType::Disabled:
                    s = "Disabled";
                    break;

            }
            ImGui::LabelText("Blending","%s",s.c_str());
            ImGui::LabelText("Depth test","%s",shader->isDepthTest()?"true":"false");
            ImGui::LabelText("Depth write","%s",shader->isDepthWrite()?"true":"false");
            ImGui::LabelText("Offset","factor: %.1f units: %.1f",shader->getOffset().x,shader->getOffset().y);

            initFramebuffer();

            auto mat = shader->createMaterial();

            static auto mesh = Mesh::create().withSphere().withName("Preview Shader Mesh").build();

            Camera camera;
            camera.setPerspectiveProjection(60,0.1,10);
            camera.lookAt({0,0,4},{0,0,0},{0,1,0});
            auto offscreenTexture = getTmpTexture();
            framebuffer->setColorTexture(offscreenTexture);
            auto renderToTexturePass = RenderPass::create()
                    .withCamera(camera)
                    .withWorldLights(&worldLights)
                    .withFramebuffer(framebuffer)
                    .withClearColor(true, {0, 0, 0, 1})
                    .withGUI(false)
                    .build();
            float rotationSpeed = 0.001f;

            renderToTexturePass.draw(mesh, glm::eulerAngleY(time*rotationSpeed), mat);

            ImGui::Image(reinterpret_cast<ImTextureID>(offscreenTexture->textureId), ImVec2(previewSize, previewSize),{0,1},{1,0},{1,1,1,1},{0,0,0,1});
            ImGui::TreePop();
        }
    }

    std::string glUniformToString(UniformType type) {
        switch (type){
            case UniformType::Float:
                return "float";
            case UniformType::Int:
                return "int";
            case UniformType::Mat3:
                return "mat3";
            case UniformType::Mat4:
                return "mat4";
            case UniformType::Texture:
                return "texture";
            case UniformType::TextureCube:
                return "texture cube";
            case UniformType::Vec3:
                return "vec3";
            case UniformType::Vec4:
                return "vec4";
            case UniformType::IVec4:
                return "ivec4";
            case UniformType::Invalid:
                return "Unsupported";

        }
        return "Unknown";
}

    void Inspector::showFramebufferObject(Framebuffer* fbo){
        std::string s = fbo->getName()+"##"+std::to_string((int64_t)fbo);
        if (ImGui::TreeNode(s.c_str())){
            char name[128];
            sprintf(name, "Color textures %i",fbo->textures.size());
            if (ImGui::TreeNode(name)){
                for (auto & t : fbo->textures){
                    showTexture(t.get());
                }
                ImGui::TreePop();
            }

            sprintf(name, "Depth textures %i",fbo->depthTexture.get()?1:0);
            if (ImGui::TreeNode(name) && fbo->depthTexture.get()){
                showTexture(fbo->depthTexture.get());
                ImGui::TreePop();
            }
            if (!fbo->depthTexture.get()){
                ImGui::LabelText("RenderBuffer Depth","%s",fbo->renderBufferDepth?"true":"false");
            }
            ImGui::TreePop();
        }
    }

    void Inspector::gui(bool useWindow) {
        Renderer* r = Renderer::instance;
        if (useWindow){
            static bool open = true;
            ImGui::Begin("SRE Renderer",&open);
        }

        if (ImGui::CollapsingHeader("Renderer")){

            ImGui::LabelText("SRE Version", "%d.%d.%d",r->sre_version_major, r->sre_version_minor, r->sre_version_point);
            if (SDLRenderer::instance){
                ImGui::LabelText("Fullscreen", "%s",SDLRenderer::instance->isFullscreen()?"true":"false");
                ImGui::LabelText("Mouse cursor locked", "%s",SDLRenderer::instance->isMouseCursorLocked()?"true":"false");
                ImGui::LabelText("Mouse cursor visible", "%s",SDLRenderer::instance->isMouseCursorVisible()?"true":"false");
            }
            ImGui::LabelText("Window size", "%ix%i",r->getWindowSize().x,r->getWindowSize().y);
            ImGui::LabelText("Drawable size", "%ix%i",r->getDrawableSize().x,r->getDrawableSize().y);
            ImGui::LabelText("VSync", "%s", r->usesVSync()?"true":"false");

            ImGui::LabelText("OpenGL version","%s (%i.%i%s)",
                             renderInfo().graphicsAPIVersion.c_str(),
                             renderInfo().graphicsAPIVersionMajor,
                             renderInfo().graphicsAPIVersionMinor,
                             renderInfo().graphicsAPIVersionES?" ES":"");

            ImGui::LabelText("OpenGL vendor", renderInfo().graphicsAPIVendor.c_str());

            SDL_version compiled;
            SDL_version linked;

            SDL_VERSION(&compiled);
            SDL_GetVersion(&linked);
            ImGui::LabelText("SDL version compiled", "%d.%d.%d",compiled.major, compiled.minor, compiled.patch);
            ImGui::LabelText("SDL version linked", "%d.%d.%d",linked.major, linked.minor, linked.patch);

            linked = *IMG_Linked_Version();
            SDL_IMAGE_VERSION(&compiled);
            ImGui::LabelText("SDL_IMG version compiled","%d.%d.%d",
                   compiled.major,
                   compiled.minor,
                   compiled.patch);
            ImGui::LabelText("SDL_IMG version linked", "%d.%d.%d",
                             linked.major, linked.minor, linked.patch);
            ImGui::LabelText("IMGUI version", IMGUI_VERSION);
        }

        if (ImGui::CollapsingHeader("Performance")){
            if (SDLRenderer::instance){
                plotTimings(millisecondsEvent.data(), "Event ms");
                plotTimings(millisecondsUpdate.data(), "Update ms");
                plotTimings(millisecondsRender.data(), "Render ms");
            }

            float max = 0;
            float sum = 0;
            for (int i=0;i<frames;i++){
                int idx = (frameCount + i)%frames;
                float t = stats[idx].drawCalls;
                data[(-frameCount%frames+idx+frames)%frames] = t;
                max = std::max(max, t);
                sum += t;
            }
            float avg = 0;
            if (frameCount > 0){
                avg = sum / std::min(frameCount, frames);
            }
            char res[128];
            sprintf(res,"Avg: %4.1f\n"
                        "Max: %4.1f\n"
                        "Cur: %4.1f",avg,max,data[frames-1]);

            ImGui::PlotLines(res,data.data(),frames, 0, "Draw calls", -1,max*1.2f,ImVec2(ImGui::CalcItemWidth(),150));

            max = 0;
            sum = 0;
            for (int i=0;i<frames;i++){
                int idx = (frameCount + i)%frames;
                float t = stats[idx].stateChangesShader + stats[idx].stateChangesMaterial + stats[idx].stateChangesMesh;
                data[(-frameCount%frames+idx+frames)%frames] = t;
                max = std::max(max, t);
                sum += t;
            }
            avg = 0;
            if (frameCount > 0){
                avg = sum / std::min(frameCount, frames);
            }
            sprintf(res,"Avg: %4.1f\n"
                        "Max: %4.1f\n"
                        "Cur: %4.1f\n"
                              ,avg,max,data[frames-1]);

            ImGui::PlotLines(res,data.data(),frames, 0, "State changes", -1,max*1.2f,ImVec2(ImGui::CalcItemWidth(),150));

            plotTimings(millisecondsFrameTime.data(), "Frame-time ms");
        }
        if (ImGui::CollapsingHeader("Memory")){
            float max = 0;
            float sum = 0;
            for (int i=0;i<frames;i++){
                int idx = (frameCount + i)%frames;
                float t = stats[idx].meshBytes/1000000.0f;
                data[(-frameCount%frames+idx+frames)%frames] = t;
                max = std::max(max, t);
                sum += t;
            }
            float avg = 0;
            if (frameCount > 0){
                avg = sum / std::min(frameCount, frames);
            }
            char res[128];
            sprintf(res,"Avg: %4.1f MB\n"
                        "Max: %4.1f MB\n"
                        "Cur: %4.1f MB\n"
                        "Count: %i",avg,max,  data[frames-1],(int)r->meshes.size());

            ImGui::PlotLines(res,data.data(),frames, 0, "Mesh MB", -1,max*1.2f,ImVec2(ImGui::CalcItemWidth(),150));

            max = 0;
            sum = 0;
            for (int i=0;i<frames;i++){
                int idx = (frameCount + i)%frames;
                float t = stats[idx].textureBytes/1000000.0f;
                data[(-frameCount%frames+idx+frames)%frames] = t;
                max = std::max(max, t);
                sum += t;
            }
            avg = 0;
            if (frameCount > 0){
                avg = sum / std::min(frameCount, frames);
            }
            sprintf(res,"Avg: %4.1f MB\n"
                        "Max: %4.1f MB\n"
                        "Cur: %4.1f MB\n"
                        "Count: %i",avg,max, data[frames-1],(int)r->textures.size());

            ImGui::PlotLines(res,data.data(),frames, 0, "Texture MB", -1,max*1.2f,ImVec2(ImGui::CalcItemWidth(),150));
        }
        if (ImGui::CollapsingHeader("Shaders")){
            for (auto s : r->shaders){
                showShader(s);
            }
            if (r->shaders.empty()){
                ImGui::LabelText("","No shaders");
            }
        }
        if (ImGui::CollapsingHeader("Textures")){
            for (auto t : r->textures){
                showTexture(t);
            }
            if (r->textures.empty()){
                ImGui::LabelText("","No textures");
            }
        }
        if (ImGui::CollapsingHeader("Meshes")){
            for (auto m : r->meshes){
                showMesh(m);
            }
            if (r->meshes.empty()){
                ImGui::LabelText("","No meshes");
            }
        }
        if (!r->spriteAtlases.empty()){
            if (ImGui::CollapsingHeader("Sprite atlases")){
                for (auto atlas : r->spriteAtlases){
                    showSpriteAtlas(atlas);
                }
            }
        }
        if (!r->framebufferObjects.empty()) {
            if (ImGui::CollapsingHeader("Framebuffer objects")) {
                for (auto fbo : r->framebufferObjects) {
                    showFramebufferObject(fbo);
                }
            }
        }
        if (useWindow) {
            ImGui::End();
        }

        if (auto shaderEditPtr = shaderEdit.lock()){
            editShader(shaderEditPtr.get());
        }
    }

    void Inspector::plotTimings(float *inputData, const char *title)  {
        float max = 0;
        float sum = 0;
        for (int i=0; i < frames; i++){
            int idx = (frameCount + i) % frames;
            float t = inputData[idx];
            data[(-frameCount % frames + idx + frames) % frames] = t;
            max = std::max(max, t);
            sum += t;
        }
        float avg = 0;
        if (frameCount > 0){
            avg = sum / std::min(frameCount, frames);
        }
        char res[128];
        sprintf(res,"Avg time: %4.2f ms\n"
                    "Max time: %4.2f ms\n"
                    "Cur time: %4.2f ms",avg,max, data[frames-1]);

        ImGui::PlotLines(res, data.data(), frames, 0, title, -1, max * 1.2f, ImVec2(ImGui::CalcItemWidth(), 150));
    }

    void updateErrorMarkers(std::vector<std::string>& errors, TextEditor& textEditor, ShaderType type){
        TextEditor::ErrorMarkers errorMarkers;
        std::regex e ( "\\d+:(\\d+)", std::regex::ECMAScript);

        std::smatch m;

        for (auto err : errors){
            auto trimmedStr = err;
            auto idx = err.find("##");
            int filter = -1;
            if (idx > 0){
                trimmedStr = err.substr(0,idx);
                auto filterStr = err.substr(idx+2);
                filter = std::stoi(filterStr);
            }
            if (filter == to_id(type)){
                int line = 0;
                if (std::regex_search (trimmedStr,m,e)) {
                    std::string match = m[1];
                    line = std::stoi(match);
                }
                errorMarkers.insert(std::pair<int, std::string>(line, trimmedStr));
            }
        }
        textEditor.SetErrorMarkers(errorMarkers);
    }

    void Inspector::editShader(Shader* shader){
        static Shader* shaderRef = nullptr;
        static std::vector<std::string> shaderCode;
        static std::vector<std::string> errors;
        static std::string errorsStr;
        static TextEditor textEditor;
        static int selectedShader = 0;
        static bool showPrecompiled = false;
        static std::vector<const char*> activeShaders;
        static std::vector<ShaderType> shaderTypes;

        if (shaderRef != shader){
            shaderRef = shader;
            shaderCode.clear();
            for (auto source : shader->shaderSources){
                auto source_ = Shader::getSource(source.second);
                shaderCode.emplace_back(source_);
            }
            selectedShader = 0;
            textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::GLSL());
            textEditor.SetText(shaderCode[selectedShader]);
            textEditor.SetPalette(TextEditor::GetDarkPalette());
            showPrecompiled = false;
            errors.clear();
            errorsStr = "";
            textEditor.SetErrorMarkers(TextEditor::ErrorMarkers());
            activeShaders.clear();
            shaderTypes.clear();
            for (auto source : shader->shaderSources){
                shaderTypes.push_back(source.first);
                switch (source.first){
                    case ShaderType::Vertex:
                        activeShaders.push_back("Vertex");
                        break;
                    case ShaderType::Fragment:
                        activeShaders.push_back("Fragment");
                        break;
                    case ShaderType::Geometry:
                        activeShaders.push_back("Geometry");
                        break;
                    case ShaderType::TessellationControl:
                        activeShaders.push_back("TessellationControl");
                        break;
                    case ShaderType::TessellationEvaluation:
                        activeShaders.push_back("TessellationEvaluation");
                        break;
                    case ShaderType::NumberOfShaderTypes:
                        LOG_ERROR("ShaderType::NumberOfShaderTypes should never be used");
                        break;
                    default:
                        LOG_ERROR("Unhandled shader");
                        break;
                }
            }
        }
        bool open = true;
        ImGui::PushID(shader);
        ImGui::Begin(shader->name.c_str(),&open);

        ImGui::PushItemWidth(-1); // align to right
        int lastSelectedShader = selectedShader;
        bool updatedShader = ImGui::Combo("####ShaderType", &selectedShader, activeShaders.data(), static_cast<int>(activeShaders.size()));
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("CTRL+1, CTRL+2, ...");
        ImGuiIO& io = ImGui::GetIO();
        if (io.KeyCtrl){
            for (int i=SDLK_1;i<SDLK_9;i++){
               if (ImGui::IsKeyPressed(i)){
                   selectedShader = i-SDLK_1;
                   updatedShader = true;
               }
            }
        }
        selectedShader = std::min(selectedShader, (int)activeShaders.size());

        bool updatedPrecompile = ImGui::Checkbox("Show precompiled", &showPrecompiled); ImGui::SameLine();
        if (updatedPrecompile){
            textEditor.SetPalette(showPrecompiled? TextEditor::GetLightPalette():TextEditor::GetDarkPalette());
        }
        bool compile = ImGui::Button("Compile");
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("CTRL+S");

        if (io.KeyCtrl && ImGui::IsKeyPressed(SDLK_s)){
            compile = true;
        }
        // update if compile or shader type changed or showPrecompiled is selected
        if ((compile && !showPrecompiled) || (updatedShader && !showPrecompiled) || (updatedPrecompile && showPrecompiled)){
            shaderCode[lastSelectedShader] = textEditor.GetText(); // get text before updating the editor
        }

        if (compile){
            auto builder = shader->update();
            for (int i=0;i<shaderTypes.size();i++){
                builder.withSourceString(shaderCode[i], shaderTypes[i]);
            }
            errors.clear();
            builder.build(errors);
            errorsStr = "";

            for (auto& err:errors) {
                errorsStr+=err+"\n";
            }
            updateErrorMarkers(errors,textEditor,shaderTypes[selectedShader]);
        }

        if (updatedShader || updatedPrecompile){
            if (showPrecompiled){
                std::vector<std::string> temp;
                textEditor.SetText(shader->precompile(shaderCode[selectedShader],temp, to_id(shaderTypes[selectedShader])));
                textEditor.SetReadOnly(true);
                textEditor.SetErrorMarkers(TextEditor::ErrorMarkers());
            } else {
                textEditor.SetText(shaderCode[selectedShader]);
                textEditor.SetReadOnly(false);
                updateErrorMarkers(errors,textEditor,shaderTypes[selectedShader]);
            }
        }
        // Show error messages
        if (!errorsStr.empty()){
            if (ImGui::CollapsingHeader("Warnings / Errors")){
                for (int i=0;i<errors.size();i++){
                    std::string id = std::string("##_errors_")+std::to_string(i);
                    ImGui::LabelText(id.c_str(), errors[i].c_str());
                }
            }
        }
        textEditor.Render("##editor");

        ImGui::End();
        ImGui::PopID();

        if (!open) {
            shaderEdit.reset();
        }
    }

    void Inspector::update() {
        usedTextures = 0;
        auto tick = Clock::now();
        float deltaTime = std::chrono::duration_cast<Milliseconds>(tick - lastTick).count();
        time += deltaTime;
        lastTick = tick;

        stats[frameCount%frames] = Renderer::instance->getRenderStats();
        millisecondsFrameTime[frameCount%frames] = deltaTime;
        if (SDLRenderer::instance){
            millisecondsEvent[frameCount%frames] = SDLRenderer::instance->deltaTimeEvent;
            millisecondsUpdate[frameCount%frames] = SDLRenderer::instance->deltaTimeUpdate;
            millisecondsRender[frameCount%frames] = SDLRenderer::instance->deltaTimeRender;
        }

        frameCount++;
    }

    void Inspector::showSpriteAtlas(SpriteAtlas *pAtlas) {
        std::string s = pAtlas->getAtlasName()+"##"+std::to_string((int64_t)pAtlas);
        if (ImGui::TreeNode(s.c_str())){
            std::stringstream ss;
            for (auto& str : pAtlas->getNames()){
                ss<< str<<'\0';
            }
            ss << '\0';
            auto ss_str = ss.str();
            static std::map<SpriteAtlas *,int> spriteAtlasSelection;
            auto elem = spriteAtlasSelection.find(pAtlas);
            if (elem == spriteAtlasSelection.end()){
                spriteAtlasSelection.insert({pAtlas, -1});
                elem = spriteAtlasSelection.find(pAtlas);
            }
            int* index = &elem->second;
            ImGui::Combo("Sprite names", index, ss_str.c_str());

            if (*index != -1){
                auto name = pAtlas->getNames()[*index];
                Sprite sprite = pAtlas->get(name);
                ImGui::LabelText("Sprite anchor","(%.2f,%.2f)",sprite.getSpriteAnchor().x,sprite.getSpriteAnchor().y);
                ImGui::LabelText("Sprite size","%ix%i",sprite.getSpriteSize().x,sprite.getSpriteSize().y);
                ImGui::LabelText("Sprite pos","(%i,%i)",sprite.getSpritePos().x,sprite.getSpritePos().y);
                auto tex = sprite.texture;
                auto uv0 = ImVec2((sprite.getSpritePos().x)/(float)tex->getWidth(), (sprite.getSpritePos().y+sprite.getSpriteSize().y)/(float)tex->getHeight());
                auto uv1 = ImVec2((sprite.getSpritePos().x+sprite.getSpriteSize().x)/(float)tex->getWidth(),(sprite.getSpritePos().y)/(float)tex->getHeight());
                ImGui::Image(reinterpret_cast<ImTextureID>(tex->textureId), ImVec2(previewSize/sprite.getSpriteSize().y*(float)sprite.getSpriteSize().x, previewSize),uv0, uv1,{1,1,1,1},{0,0,0,1});
            }

            ImGui::TreePop();
        }
    }

    void Inspector::initFramebuffer() {
        if (framebuffer == nullptr){
            framebuffer = Framebuffer::create().withColorTexture(getTmpTexture()).withName("SRE Inspector Framebufferobject").build();
            usedTextures = 0; // reset usedTextures count to avoid an extra texture to be created
            worldLights.setAmbientLight({0.2,0.2,0.2});
            auto light = Light::create().withPointLight({0,0,4}).build();
            worldLights.addLight(light);
        }
    }

    std::shared_ptr<Texture> Inspector::getTmpTexture() {
        if (usedTextures < offscreenTextures.size()){
            int index = usedTextures;
            usedTextures++;
            return offscreenTextures[index];
        }
        auto offscreenTex = Texture::create().withRGBData(nullptr, 256,256).withName(std::string("SRE Inspector Tex #")+std::to_string(offscreenTextures.size())).build();
        offscreenTextures.push_back(offscreenTex);
        usedTextures++;
        return offscreenTex;
    }
}