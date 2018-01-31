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

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/Inspector.hpp>

using namespace sre;

class MultipleLightsExample {
public:
    MultipleLightsExample (){
        r.init();

        camera.lookAt(eye,at, up);
        camera.setPerspectiveProjection(60,0.1f,100);

        meshSphere = Mesh::create().withSphere().build();
        meshCube = Mesh::create().withCube().build();

        for (int i=0;i<Renderer::instance->getMaxSceneLights();i++){
            worldLights.addLight(Light::create().withPointLight({0, 2,1}).withColor({1,1,1}).withRange(10).build());
        }
        mat = Shader::getStandardBlinnPhong()->createMaterial();
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
        if (animatedObject){
            rot += deltaTime;
        }
    }
    void render() {
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1,0,0,1})
                .build();
        drawCross(renderPass,{2,2,2});
        drawCross(renderPass,{-2,-2,-2});


        renderPass.draw(drawSphere?meshSphere:meshCube, glm::eulerAngleY(rot), mat);

        ImGui::DragFloat3("Camera",&eye.x);
        ImGui::Checkbox("AnimatedLight",&animatedLight);
        ImGui::Checkbox("AnimatedCamera",&animatedCamera);
        ImGui::Checkbox("AnimatedObject",&animatedObject);

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
        ImGui::Checkbox("Draw Sphere",&drawSphere);

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
            if (ImGui::Checkbox("BlinnPhong",&useBlinnPhong)){
                mat = useBlinnPhong?Shader::getStandardBlinnPhong()->createMaterial() : Shader::getStandardPBR()->createMaterial();
            }
            if (useBlinnPhong){
                ImGui::DragFloat4("Specularity", &specularity.r,0.1,0,1);
                mat->setSpecularity(specularity);
            } else {
                ImGui::DragFloat("Metallic", &metalRoughness.x,0.1,0,1);
                ImGui::DragFloat("Roughness", &metalRoughness.y,0.1,0,1);
                mat->setMetallicRoughness(metalRoughness);
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
    glm::vec3 eye{0,0,5};
    glm::vec3 at{0,0,0};
    glm::vec3 up{0,1,0};
    Camera camera;

    std::shared_ptr<Material> mat;
    bool useBlinnPhong = true;
    glm::vec2 metalRoughness;
    Color specularity = Color(1,1,1,20);
    sre::Color color {1,1,1,1};

    std::shared_ptr<Mesh> meshSphere;
    std::shared_ptr<Mesh> meshCube;

    bool drawSphere = true;

    WorldLights worldLights;

    bool debugLight = true;
    bool animatedLight = true;
    bool animatedCamera = true;
    bool animatedObject = true;

    float rot = 0;

    float debugLightSize = 0.2;
    float time = 0;
};

int main() {
    new MultipleLightsExample();

    return 0;
}
