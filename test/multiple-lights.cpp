#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Material.hpp"
#include "sre/Mesh.hpp"
#include "sre/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"
#include <imgui.h>
#include "sre/imgui_sre.hpp"

#include <glm/glm.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class MultipleLightsExample {
    public:
    MultipleLightsExample (){
        r.init();

        camera.lookAt(eye,at, up);
        camera.setPerspectiveProjection(60,0.1f,100);

        mesh = Mesh::create().withSphere().build();

        worldLights.addLight(Light::create().withPointLight({0, 2,1}).withColor({1,0,0}).withRange(10).build());
        worldLights.addLight(Light::create().withPointLight({0, 2,1}).withColor({1,0,0}).withRange(10).build());
        worldLights.addLight(Light::create().withPointLight({0, 2,1}).withColor({1,0,0}).withRange(10).build());
        worldLights.addLight(Light::create().withPointLight({0, 2,1}).withColor({1,0,0}).withRange(10).build());

        mat = Shader::getStandard()->createMaterial();
        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.frameRender = [&](){
            render();
        };
        r.startEventLoop();
    }


    void update(float deltaTime) {
        mat->setSpecularity(specularity);
        mat->setColor(color);

        time += deltaTime;
    }
    void render() {
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1,0,0,1})
                .build();
        drawCross(renderPass,{2,2,2});
        drawCross(renderPass,{-2,-2,-2});
        renderPass.draw(mesh, glm::eulerAngleY(time), mat);


        std::string labels[] = {
                "Light 1",
                "Light 2",
                "Light 3",
                "Light 4"
        };

        ImGui::DragFloat3("Camera",&eye.x);
        ImGui::Checkbox("AnimatedLight",&animatedLight);
        ImGui::Checkbox("AnimatedCamera",&animatedCamera);
        if (animatedCamera){
            eye = {
                    sin(time*-0.2)*5.0f,
                    sin(time*-0.4)*0.5f,
                    cos(time*-0.2)*5.0f,
            };
        }

        camera.lookAt(eye,at, up);
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
        // Show Label (with invisible window)
        for (int i=0;i<4;i++){
            auto l = worldLights.getLight(i);
            if (debugLight){
                drawLight(renderPass,l,debugLightSize);
            }
            if (ImGui::TreeNode(labels[i].c_str())){
                auto lightType = (int)l->lightType;
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
            ImGui::DragFloat("Specularity", &specularity,1,0,200);
            mat->setSpecularity(specularity);
            ImGui::ColorEdit3("Color", &(color.x));
            ImGui::TreePop();
        }
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
    glm::vec3 eye{0,0,5};
    glm::vec3 at{0,0,0};
    glm::vec3 up{0,1,0};
    Camera camera;

    std::shared_ptr<Material> mat;
    float specularity = 20;
    glm::vec4 color {1,1,1,1};

    std::shared_ptr<Mesh> mesh;

    WorldLights worldLights;

    bool debugLight = true;
    bool animatedLight = true;
    bool animatedCamera = true;

    float debugLightSize = 0.2;
    float time = 0;
};

int main() {
    new MultipleLightsExample();

    return 0;
}

