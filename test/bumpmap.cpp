#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>
#include <sre/Inspector.hpp>

using namespace sre;

class BumpMap{
public:
    BumpMap(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1f,100);

        mesh = Mesh::create()
                .withSphere()
                .build();

        worldLights.addLight(Light::create()
                                     .withDirectionalLight(glm::normalize(glm::vec3(1,1,1)))
                                     .build());
        std::map<std::string,std::string> specialization;
        specialization["S_NORMALMAP"] = "1";
        mat1 = Shader::getStandardBlinnPhong()->createMaterial(specialization);
        mat1->setColor({1,1,1,1});
        mat1->setSpecularity(Color(1,1,1,50));
        mat1->set("normalTex", Texture::create().withFile("test_data/normal.jpg").withSamplerColorspace(Texture::SamplerColorspace::Gamma).build());
        mat1->set("normalScale", 1.0f);
        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.frameRender = [&](){
            render();
        };

        r.mouseEvent = [&](SDL_Event& event) {
            if (event.button.button == SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.startEventLoop();
    }

    void update(float deltaTime){
        time += deltaTime;
    }

    void render(){
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true,{1,0,0,1})
                .build();

        rp.draw(mesh, glm::rotate(time, glm::vec3(0,1,0))*pos1, mat1);


        static sre::Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }

        checkGLError();

    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    WorldLights worldLights;
    std::shared_ptr<Material> mat1;
    bool showInspector = false;
    glm::mat4 pos1 = glm::translate(glm::mat4(1), {0,0,0});
};

int main() {
    std::make_unique<BumpMap>();
    return 0;
}

