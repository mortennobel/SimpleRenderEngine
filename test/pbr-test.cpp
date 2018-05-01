//
// Created by Morten Nobel-Jørgensen on 18/07/2017.
//

#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/SDLRenderer.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <sre/SpriteAtlas.hpp>
#include <sre/Inspector.hpp>

using namespace sre;

class PBRTest {
public:
    glm::vec3 rotatedPosition(float x, float y, float distance){
        glm::vec3 pos{0,0,distance};
        return glm::rotateY(glm::rotateX(pos, glm::radians(x)),glm::radians(y));
    }

    std::shared_ptr<Texture> updateTexture(char* filename, std::shared_ptr<Texture> fallback){
        auto res = Texture::create().withFile(filename).build();
        if (res){
            return res;
        }
        return fallback;
    }

    PBRTest()
    {
        r.setWindowTitle("Physically Based Rendering");
        r.init();

        updateLight();
        camera.setPerspectiveProjection(60,0.1,100);
        camera.lookAt({0,0,cameraDist},{0,0,0},{0,1,0});
        camera.setViewport({0.3333,0},{0.6666,1});

        colorTex = Texture::create().withWhiteData(2,2).build();
        metToughTex = Texture::create().withWhiteData(2,2).build();
        normalTex = Texture::create().withWhiteData(2,2).build();
        emissiveTex = Texture::create().withWhiteData(2,2).build();
        occlusionTex = Texture::create().withWhiteData(2,2).build();
        colorTex     =      Texture::create().withFile(colorTexStr    ).build();
        metToughTex =   Texture::create().withFile(metRoughTexStr )
                .withSamplerColorspace(Texture::SamplerColorspace::Gamma)
                .build();
        normalTex =     Texture::create()
                .withFile(normalTexStr   )
                .withSamplerColorspace(Texture::SamplerColorspace::Gamma)
                .build();
        emissiveTex =   Texture::create().withFile(emissiveTexStr ).build();
        occlusionTex =  Texture::create().withFile(occlusionTexStr)
                .withSamplerColorspace(Texture::SamplerColorspace::Gamma)
                .build();

        updateMaterial();

        meshes = {{
            Mesh::create().withSphere(32,64).build(),
            Mesh::create().withCube().build(),
            Mesh::create().withTorus(48,48).build()
        }};

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event event){
            static bool validClick = false;
            if (event.type == SDL_MOUSEBUTTONDOWN){
                validClick = !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
            }
            if (event.type == SDL_MOUSEMOTION && validClick){
                if (event.motion.state & SDL_BUTTON_LMASK){
                    float mouseSpeed = 0.8;
                    cameraRotateY += event.motion.xrel * mouseSpeed;
                    cameraRotateX += event.motion.yrel * mouseSpeed;
                    cameraRotateX = glm::clamp<float>(cameraRotateX,-179.0f,179.0f);
                    camera.lookAt(rotatedPosition(cameraRotateX,cameraRotateY,cameraDist),{0,0,0},{0,1,0});
                }
            }
        };

        r.startEventLoop();
    }

    void updateMaterial(){
        material = (pbrShader?Shader::getStandardPBR(): Shader::getStandardBlinnPhong())->createMaterial(specialization);
        material->setColor(color);
        material->setMetallicRoughness(metallicRoughness);
        material->setTexture(colorTex);
        material->set("normalTex",normalTex);
        material->set("mrTex",metToughTex);
        material->set("normalScale",normalScale);
        material->set("emissiveTex",emissiveTex);
        material->set("emissiveFactor",emissiveFactor);
        material->set("occlusionTex",occlusionTex);
        material->set("occlusionStrength",occlusionStrength);
        material->setSpecularity(specularity);
    }

    void render(){
        // updateLight
        static Camera clearScreen;
        auto renderPassClear = RenderPass::create()
                .withCamera(clearScreen)
                .withGUI(false)
                .withClearColor(true, {0,0,0,1})
                .build();
        renderPassClear.finish();

        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(lightCount==0?(&lightsSingle):(&lightsDuo))
                .withClearColor(true, {.2f,.2f,.2f,1})
                .build();
        renderPass.draw(meshes[meshType],glm::mat4(1), material);

        renderGUI();
        renderPass.finish();
    }

    bool loadTexture(std::string label, std::shared_ptr<Texture>& texRef, char* fileLocation){
        bool changed = ImGui::InputText(label.c_str(), fileLocation, maxTextSize);
        if (changed){
            auto res = Texture::create().withFile(fileLocation).build();
            if (res){
                texRef = res;
                return true;
            }
        }
        return false;
    }

    void renderGUI(){
        ImGui::SetNextWindowPos(ImVec2{0,0}); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
        ImGui::SetNextWindowSize(ImVec2{800*.3333f,600});          // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
        ImGui::Begin("PBR");
        bool updatedMat = false;
        if (ImGui::CollapsingHeader("Material")){
            auto col4 = color.toLinear();
            updatedMat |= ImGui::ColorEdit4("Color", &col4.x);
            color.setFromLinear(col4);
            if (specialization.find("S_NO_BASECOLORMAP") == specialization.end()) {
                updatedMat |= loadTexture("ColorTex", colorTex, colorTexStr);
            }
            if (pbrShader){
                updatedMat |= ImGui::DragFloat("Metallic", &metallicRoughness.x,0.05f,0,1);
                updatedMat |= ImGui::DragFloat("Roughness", &metallicRoughness.y,0.05f,0,1);
            } else {
                updatedMat |= ImGui::DragFloat4("Specularity", &specularity.r,0.1f,0,1);
            }
            if (specialization.find("S_METALROUGHNESSMAP") != specialization.end()) {
                updatedMat |= loadTexture("MetallicRoughnessTex", metToughTex,metRoughTexStr);
            }
            if (specialization.find("S_NORMALMAP") != specialization.end()) {
                updatedMat |= loadTexture("NormalTex", normalTex,normalTexStr);
                updatedMat |= ImGui::DragFloat("NormalScale", &normalScale, 0.1f, 0.0f, 2.0f);
            }
            if (specialization.find("S_OCCLUSIONMAP") != specialization.end()) {
                updatedMat |= loadTexture("Occlusion Tex", occlusionTex,occlusionTexStr);
                updatedMat |= ImGui::DragFloat("Occlusion Strength", &occlusionStrength, 0.1f, 0.0f, 2.0f);
            }
            if (specialization.find("S_EMISSIVEMAP") != specialization.end()) {
                updatedMat |= loadTexture("Emissive Tex", emissiveTex,emissiveTexStr);
                updatedMat |= ImGui::DragFloat4("Emissive Factor", &emissiveFactor.x, 0.1f, 0.0f, 2.0f);
            }
        }
        if (ImGui::CollapsingHeader("Light")){
            ImGui::Combo("Light count",&lightCount,"One light\0Two lights\0");
            bool updatedLight = false;
            updatedLight |= ImGui::Combo("Light type",&lightType,"Directional\0Point\0");
            if (lightType == 1){
                updatedLight |=  ImGui::DragFloat("LightDistance",&lightDistance, 0.5f,0.0f,50.0f);
            }
            updatedLight |= ImGui::DragFloat("Ambient light",&ambientLight, 0.02f,0.0f,1.0f);
            if (updatedLight){
                updateLight();
            }

        }
        if (ImGui::CollapsingHeader("Model")){
            ImGui::Combo("Mesh",&meshType, "Sphere\0Cube\0Torus\0");
        }
        if (ImGui::CollapsingHeader("Shader")){
            updatedMat |= ImGui::Checkbox("pbrShader", &pbrShader);
            auto shaderConstants = (pbrShader?Shader::getStandardPBR(): Shader::getStandardBlinnPhong())->getAllSpecializationConstants();
            for (auto& s : shaderConstants){
                bool checked = specialization.find(s) != specialization.end();
                if (ImGui::Checkbox(s.c_str(), &checked)){
                    if (checked){
                        specialization.insert({s,"1"});
                    } else {
                        specialization.erase(specialization.find(s));
                    }
                    updatedMat = true;
                }
            }
        }
        if (updatedMat){
            updateMaterial();
        }
        ImGui::End();

        static Inspector inspector;
        inspector.update();
        inspector.gui();
    }

    void updateLight(){
        lightsSingle.clear();
        lightsDuo.clear();
        lightsSingle.setAmbientLight({ambientLight,ambientLight,ambientLight});
        lightsDuo.setAmbientLight({ambientLight,ambientLight,ambientLight});
        bool directional = lightType==0;
        if (directional ){
            lightsSingle.addLight(Light::create().withDirectionalLight(rotatedPosition(-10,-10,1)).withColor({1,1,1}).build());
            lightsDuo.addLight(Light::create().withDirectionalLight(rotatedPosition(-30,-30,1)).withColor({1,1,1}).build());
            lightsDuo.addLight(Light::create().withDirectionalLight(rotatedPosition(30,-30+180,1)).withColor({1,1,1}).build());
        } else {
            lightsSingle.addLight(Light::create().withPointLight(rotatedPosition(-10,-10,lightDistance)).withColor({1,1,1}).build());
            lightsDuo.addLight(Light::create().withPointLight(rotatedPosition(-30,-30,lightDistance)).withColor({1,1,1}).build());
            lightsDuo.addLight(Light::create().withPointLight(rotatedPosition(30,-30+180,lightDistance)).withColor({1,1,1}).build());
        }
    }
private:
    SDLRenderer r;
    Camera camera;
    bool pbrShader = true;
    static constexpr float cameraDist = 3.5f;
    float cameraRotateX = 0;
    float cameraRotateY = 0;
    int selection = 0;
    Color color = {1,1,1,1};
    const static int maxTextSize = 512;
    char colorTexStr[maxTextSize]    = "test_data/BoomBox_baseColor.png";
    char metRoughTexStr[maxTextSize] = "test_data/BoomBox_roughnessMetallic.png";
    char normalTexStr[maxTextSize]   = "test_data/BoomBox_normal.png";
    float normalScale = 1;
    char emissiveTexStr[maxTextSize] = "test_data/BoomBox_emissive.png";
    glm::vec4 emissiveFactor = glm::vec4(1,1,1,1);
    char occlusionTexStr[maxTextSize] ="test_data/BoomBox_occlusion.png";
    float occlusionStrength = 1;
    std::shared_ptr<sre::Texture> colorTex;
    std::shared_ptr<sre::Texture> metToughTex;
    std::shared_ptr<sre::Texture> normalTex;
    std::shared_ptr<sre::Texture> emissiveTex;
    std::shared_ptr<sre::Texture> occlusionTex;
    std::map<std::string,std::string> specialization;
    glm::vec2 metallicRoughness = glm::vec2(0.0,0.5);
    Color specularity = Color(1,1,1,50);

    int meshType = 0;
    std::vector<std::shared_ptr<sre::Mesh>> meshes;

    int lightCount = 0;
    float lightDistance = 10;
    float ambientLight = .1f;
    int lightType = 0;
    WorldLights lightsSingle;
    WorldLights lightsDuo;

    std::shared_ptr<Material> material;
};

int main() {
    PBRTest pbrTest;
    return 0;
}
