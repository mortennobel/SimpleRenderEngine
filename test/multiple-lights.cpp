#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Material.hpp"
#include "sre/Mesh.hpp"
#include "sre/Shader.hpp"
#include <imgui.h>
#include "sre/imgui_sre.hpp"

#include <glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/Inspector.hpp>
#include <sre/ModelImporter.hpp>

using namespace sre;

class MultipleLightsExample {
public:
    MultipleLightsExample (){
        r.init();

        camera.setPerspectiveProjection(60,0.1f,100);

        meshes[0] = Mesh::create().withSphere().build();
        meshes[1] = Mesh::create().withCube().build();
        meshes[2]= sre::ModelImporter::importObj("examples_data/", "suzanne.obj");

        worldLights.setAmbientLight({0.1,0.1,0.1});
        for (int i=0;i<Renderer::instance->getMaxSceneLights();i++) {
            worldLights.addLight(Light::create().withPointLight({0, 2,1}).withColor({1,1,1}).withRange(10).build());
        }
        mat[0] = Shader::getStandardBlinnPhong()->createMaterial();
        mat[1] = Shader::getStandardPhong()->createMaterial();
        mat[2] = Shader::getStandardPBR()->createMaterial();
        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEMOTION){
                SDL_MouseMotionEvent motion = event.motion;
                float rotationSpeed = 0.15f;
                auto windowSize = Renderer::instance->getWindowSize();

                camRotX = (motion.y / (float)windowSize.y-0.5f)*glm::pi<float>();
                camRotY = (motion.x / (float)windowSize.x)*glm::pi<float>()*2;
            } else if (event.type == SDL_MOUSEWHEEL){
                float wheelSpeed = 0.15f;
                eyeRadius+= event.wheel.y *wheelSpeed;
            }
        };
        r.keyEvent = [&](SDL_Event& event){
            if (event.type == SDL_KEYDOWN){
                if (event.key.keysym.sym == SDLK_r){
                    rotateCamera = !rotateCamera;
                }
            }
        };
        r.startEventLoop();
    }

    void update(float deltaTime) {
        mat[selectedMaterial]->setSpecularity(specularity);
        mat[selectedMaterial]->setColor(color);

        time += deltaTime;
        if (animatedObject){
            rot += deltaTime;
        }
    }
    void render() {
        glm::vec4 eye =glm::rotate(camRotY,glm::vec3(0,1,0))*glm::rotate(camRotX, glm::vec3(1,0,0)) * glm::vec4(0,0,eyeRadius,1);
        camera.lookAt(eye, {0,0,0},{0,1,0});

        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {0.2f,0.2f,0.2f,1})
                .build();

        if (!multipleModels){
            renderPass.draw(meshes[meshId], glm::eulerAngleY(rot), mat[selectedMaterial]);
        } else {
            for (int x=-5;x<=5;x = x +2){
                for (int z=-5;z<=5;z= z + 2){
                    renderPass.draw(meshes[meshId], glm::translate(glm::vec3{x,0,z})*glm::eulerAngleY(rot), mat[selectedMaterial]);
                }
            }
        }

        ImGui::DragFloat2("Eye",&eye.x);
        ImGui::Checkbox("AnimatedLight",&animatedLight);
        ImGui::Checkbox("AnimatedObject",&animatedObject);
        ImGui::Checkbox("Multiple object",&multipleModels);
        ImGui::Checkbox("Rotate camera",&rotateCamera);
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("'r'-key to toogle camera rotate");
        }

        if (animatedLight){
            worldLights.getLight(0)->position = {
                    sin(time)*1.5f,
                    sin(time*2)*0.5f,
                    cos(time)*1.5f,
            };
        }
        ImGui::Checkbox("DebugLight",&debugLight);
        if (debugLight) {
            ImGui::DragFloat("DebugLightSize", &debugLightSize,0.1f,0,3);
        }

        ImGui::Combo("Mesh",&meshId, "Sphere\0Cube\0Suzanne\0");
        auto ambientLight = worldLights.getAmbientLight();
        ImGui::DragFloat3("Ambient light", &ambientLight.x,0.01,0,1);
        worldLights.setAmbientLight(ambientLight);
        // Show Label (with invisible window)
        for (int i=0;i<Renderer::instance->getMaxSceneLights();i++){
            auto l = worldLights.getLight(i);
            if (debugLight){
                drawLight(renderPass,l,debugLightSize);
            }
            std::string lightLabel = "Light ";
            lightLabel += std::to_string(i+1);
            if (ImGui::TreeNode(lightLabel.c_str())){
                auto lightType = (int)l->lightType;
                ImGui::SameLine();
                ImGui::RadioButton("Point", &lightType, 0); ImGui::SameLine();
                ImGui::RadioButton("Directional", &lightType, 1); ImGui::SameLine();
                ImGui::RadioButton("Unused", &lightType, 2);
                l->lightType = (LightType)lightType;

                ImGui::ColorEdit3("Color", &(l->color.x));
                ImGui::DragFloat3("Position", &(l->position.x));
                ImGui::DragFloat3("Direction", &(l->direction.x));
                ImGui::DragFloat("Range", &(l->range),1,0,30);
                ImGui::TreePop();
            }
        }

        if (ImGui::TreeNode("Material")){
            const char* options = "BlinnPhong\0Phong\0PBR\0";
            ImGui::Combo("Shader",&selectedMaterial,options);

            if (selectedMaterial<2){
                ImGui::DragFloat4("Specularity", &specularity.r,0.1,0,1);
                mat[selectedMaterial]->setSpecularity(specularity);
            } else {
                ImGui::DragFloat("Metallic", &metalRoughness.x,0.1,0,1);
                ImGui::DragFloat("Roughness", &metalRoughness.y,0.1,0,1);
                mat[selectedMaterial]->setMetallicRoughness(metalRoughness);
            }
            auto col = color.toLinear();
            if (ImGui::ColorEdit3("Color", &(col.x))){
                color.setFromLinear(col);
            }
            ImGui::TreePop();
        }

        static Inspector inspector;
        inspector.update();
        inspector.gui();
    }

    void drawCross(RenderPass& rp, glm::vec3 p, float size = 0.3f){
        rp.drawLines({p-glm::vec3{size,0,0}, p+glm::vec3{size,0,0},
                      p-glm::vec3{0,size,0}, p+glm::vec3{0,size,0},
                      p-glm::vec3{0,0,size}, p+glm::vec3{0,0,size}
                     },{0,0,0,1});
    }

    void drawLight(RenderPass& rp, Light* l, float size){
        if (l->lightType == LightType::Point || l->lightType == LightType::Directional){
            drawCross(rp, l->position, size);
        }
        if (l->lightType == LightType::Directional){
            rp.drawLines({l->position, l->position - l->direction*size*2.0f},{1,1,0,1});
        }
    }

private:
    SDLRenderer r;

    Camera camera;
    int selectedMaterial = 0;
    std::shared_ptr<Material> mat[3];

    glm::vec2 metalRoughness = {.5,.5};
    Color specularity = Color(1,1,1,20);
    sre::Color color {1,1,1,1};

    std::shared_ptr<Mesh> meshes[3];
    int meshId = 0;

    bool multipleModels = false;

    WorldLights worldLights;

    bool debugLight = true;
    bool animatedLight = true;
    bool animatedObject = true;

    float rot = 0;

    float debugLightSize = 0.2;
    float time = 0;

    float camRotX = 0;
    float camRotY = glm::half_pi<float>();
    float eyeRadius = 4;

    bool rotateCamera = true;
};

int main() {
    std::make_unique<MultipleLightsExample>();
    return 0;
}
