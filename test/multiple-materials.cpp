#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>


using namespace sre;

class SpinningCubeExample {
public:
    SpinningCubeExample(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);

        materialPhong = Shader::getStandardBlinnPhong()->createMaterial();
        materialPhong->setColor({1.0f,1.0f,1.0f,1.0f});
        materialPhong->setSpecularity(Color(.5,.5,.5,180.0f));
        materialPhongRed = Shader::getStandardBlinnPhong()->createMaterial();
        materialPhongRed->setColor({1.0f,0.0f,0.0f,1.0f});
        materialPhongRed->setSpecularity(Color(.5,.5,.5,180.0f));

        materialPhongGreen = Shader::getStandardBlinnPhong()->createMaterial();
        materialPhongGreen->setColor({0.0f,1.0f,0.0f,1.0f});
        materialPhongGreen->setSpecularity(Color(.5,.5,.5,180.0f));

        materials = {materialPhong,materialPhongRed};
        meshSingle = Mesh::create()
                .withCube()
                .build();
        auto indices = meshSingle->getIndices(0);
        auto indicesFirst = std::vector<uint32_t>(indices.begin()+0, indices.begin()+indices.size()/2);
        auto indicesLast = std::vector<uint32_t>(indices.begin()+indices.size()/2, indices.begin()+indices.size());

        mesh = Mesh::create()
                .withCube()
                .withIndices(indicesFirst, MeshTopology::Triangles,0)
                .withIndices(indicesLast, MeshTopology::Triangles, 1)
                .build();
        worldLights.setAmbientLight({0.0,0.0,0.0});
        worldLights.addLight(Light::create().withPointLight({0, 3,0}).withColor({1,1,1}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({3, 0,0}).withColor({1,1,1}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({0,-3,0}).withColor({1,1,1}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({-3,0,0}).withColor({1,1,1}).withRange(20).build());

        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {1, 0, 0, 1})
                .build();
        renderPass.draw(mesh, glm::eulerAngleX(glm::radians((float)i*0.3f))*glm::eulerAngleY(glm::radians((float)i*0.1f)), materials);
        renderPass.draw(meshSingle, glm::translate(glm::vec3{-1.5,-1.5,0})*glm::eulerAngleY(glm::radians((float)i*0.1f))*glm::scale(glm::vec3{0.1,0.1,0.1}), materialPhongGreen);
        i++;
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> meshSingle;
    std::shared_ptr<Material> materialPhong;
    std::shared_ptr<Material> materialPhongRed;
    std::shared_ptr<Material> materialPhongGreen;
    std::vector<std::shared_ptr<Material>> materials;
    int i=0;
};

int main() {
    std::make_unique<SpinningCubeExample>();
    return 0;
}
