#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <sre/ModelImporter.hpp>
#include <glm/gtx/string_cast.hpp>


using namespace sre;

class SpinningCubeExample {
public:
    SpinningCubeExample(){
        r.init();

        SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
        r.otherEvent = [&](SDL_Event& e){
            if (e.type == SDL_DROPFILE){
                auto dropped_filedir = e.drop.file;
                loadObjFile(dropped_filedir);
                SDL_free(dropped_filedir);    // Free dropped_filedir memory
            }
        };

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,farPlane);

        auto material = Shader::getStandard()->createMaterial();
        material->setColor({1.0f,1.0f,1.0f,1.0f});
        material->setSpecularity(20.0f);
        materials.push_back(material);

        mesh = Mesh::create().withCube().build();
        worldLights.addLight(Light::create().build());
        worldLights.addLight(Light::create().build());
        worldLights.addLight(Light::create().build());
        worldLights.addLight(Light::create().build());

        setLight(1);

        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void setLight(int config){
        if (config == 0){
            worldLights.setAmbientLight({0.05f,0.05f,0.05f});
            worldLights.getLight(0)->lightType = LightType::Point;
            worldLights.getLight(0)->color = {1,1,1};
            worldLights.getLight(0)->position = {0,0,4};
            worldLights.getLight(0)->range = 100;
            worldLights.getLight(1)->lightType = LightType::Unused;
            worldLights.getLight(2)->lightType = LightType::Unused;
            worldLights.getLight(3)->lightType = LightType::Unused;
        }
        if (config == 1){
            worldLights.setAmbientLight({0.05f,0.05f,0.05f});
            worldLights.getLight(0)->lightType = LightType::Directional;
            worldLights.getLight(0)->color = {1,1,1};
            worldLights.getLight(0)->direction = {1,1,.2};
            worldLights.getLight(1)->lightType = LightType::Directional;
            worldLights.getLight(1)->color = {0,0,.3};
            worldLights.getLight(1)->direction = {-1,-1,-.8};
            worldLights.getLight(2)->lightType = LightType::Unused;
            worldLights.getLight(3)->lightType = LightType::Unused;
        }
        if (config == 2){
            worldLights.setAmbientLight({0.05f,0.05f,0.05f});
            worldLights.getLight(0)->lightType = LightType::Directional;
            worldLights.getLight(0)->color = {1,0,0};
            worldLights.getLight(0)->direction = {1,0,0};
            worldLights.getLight(1)->lightType = LightType::Directional;
            worldLights.getLight(1)->color = {0,1,0};
            worldLights.getLight(1)->direction = {-1,0,0};
            worldLights.getLight(2)->lightType = LightType::Unused;
            worldLights.getLight(3)->lightType = LightType::Unused;
        }
        if (config == 3){
            worldLights.setAmbientLight({0.05f,0.05f,0.05f});
            worldLights.getLight(0)->lightType = LightType::Point;
            worldLights.getLight(0)->color = {1,0,0};
            worldLights.getLight(0)->position = {0, 3,0};
            worldLights.getLight(0)->range = 20;
            worldLights.getLight(1)->lightType = LightType::Point;
            worldLights.getLight(1)->color = {0,1,0};
            worldLights.getLight(1)->position = {3, 0,0};
            worldLights.getLight(1)->range = 20;
            worldLights.getLight(2)->lightType = LightType::Point;
            worldLights.getLight(2)->color = {0,-3,0};
            worldLights.getLight(2)->position = {0,-3,0};
            worldLights.getLight(2)->range = 20;
            worldLights.getLight(3)->lightType = LightType::Point;
            worldLights.getLight(3)->color = {1,1,1};
            worldLights.getLight(3)->position = {-3,0,0};
            worldLights.getLight(3)->range = 20;
        }
    }

    void loadObjFile(std::string file){
        auto pos = file.find_last_of(kPathSeparator)+1;
        auto path = file.substr(0,pos);
        auto filename = file.substr(pos);

        materials.clear();
        mesh = ModelImporter::importObj(path, filename, materials);

        std::cout<<materials.size()<< " "<<mesh->getIndexSets()<<std::endl;
        auto bounds = mesh->getBoundsMinMax();
        auto center = glm::mix(bounds[1] , bounds[0],0.5f);
        offset = -center;

        farPlane =  glm::length(bounds[1] - bounds[0]);

        camera.lookAt({0,0,farPlane/2},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,farPlane);
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {0, 0, 0, 1})
                .build();

        renderPass.draw(mesh, glm::scale(glm::vec3(0.5f))*glm::eulerAngleY(glm::radians((float)i))*glm::translate(offset), materials);

        lightGUI();

        // align text
        drawTopText(
#ifdef EMSCRIPTEN
        "Drop obj not supported in Emscripten"
#else
        "Drop obj file here"
#endif
        );

        i++;
    }

    void updateLight(Light* light,int i){
        ImGui::PushID(i);
        ImGui::LabelText("","Light index %i",i);
        const char* items[] = {"Point","Directional","None"};
        ImGui::Combo("Light type", (int*)(&light->lightType), items,3);
        if (light->lightType == LightType::Directional){
            ImGui::DragFloat3("Direction",&light->direction.x);
        }
        else if(light->lightType == LightType::Point){
            ImGui::DragFloat3("Position", &light->position.x);
            ImGui::DragFloat("Range", &light->range);
        }
        if (light->lightType != LightType::Unused){
            ImGui::ColorEdit3("Color", &light->color.x);
        }
        ImGui::PopID();
    }

    void lightGUI(){
        static bool lightOpen = false;
        ImGui::Begin("Lights", &lightOpen);
        if (ImGui::CollapsingHeader("Predefined configs")){
            if (ImGui::Button("Camera light")){
                setLight(0);
            }
            if (ImGui::Button("Twin lights")){
                setLight(1);
            }
            if (ImGui::Button("Twin lights Left-Right")){
                setLight(2);
            }
            if (ImGui::Button("Colorful")){
                setLight(3);
            }
        }

        auto ambientLight = worldLights.getAmbientLight();
        if (ImGui::ColorEdit3("Ambient light",&ambientLight.x)){
            worldLights.setAmbientLight(ambientLight);
        }
        for (int i=0;i<4;i++){
            Light* light = worldLights.getLight(i);
            updateLight(light,i);
        }
        ImGui::End();
    }

    void drawTopText(std::string txt){
        auto size = Renderer::instance->getWindowSize();
        ImVec2 imSize(size.x, 50.0f);
        ImVec2 imPos(0, 0);
        ImGui::SetNextWindowSize(imSize);                                   // imgui window size should have same width as SDL window size
        ImGui::SetNextWindowPos(imPos);
        ImGui::Begin("",nullptr,ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar);
                                                                            // create window without title
        auto txtSize = ImGui::CalcTextSize(txt.c_str());                    // center text
        ImGui::SetCursorPosX((imSize.x-txtSize.x)/2);
        ImGui::TextWrapped(txt.c_str());
        ImGui::End();
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::vector<std::shared_ptr<Material>> materials;
    int i=0;
    glm::vec3 offset{0};
    float farPlane = 100;
	const char kPathSeparator =
#ifdef _WIN32
		'\\';
#else
		'/';
#endif
};

int main() {
    new SpinningCubeExample();
    return 0;
}
