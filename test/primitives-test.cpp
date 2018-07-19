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

class PrimitivesTest {
public:
    PrimitivesTest(){
        r.init();

        camera.lookAt({0,0,3},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);

        material = Shader::getStandardBlinnPhong()->createMaterial();
        material->setColor({1.0f,1.0f,1.0f,1.0f});
        material->setSpecularity(Color(1,1,1,20.0f));

        mesh = Mesh::create().withCube().build();
        worldLights.setAmbientLight({0.0,0.0,0.0});
        worldLights.addLight(Light::create().withPointLight({0, 3,0}).withColor({1,0,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({3, 0,0}).withColor({0,1,0}).withRange(20).build());
        worldLights.addLight(Light::create().withPointLight({0,-3,0}).withColor({0,0,1}).withRange(20).build());
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
        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i*0.5f)), material);
        gui();
        i++;
    }

    void gui(){
        bool changed = ImGui::Combo("Shader",&shader,"Phong\0UVs\0Normals\0Tangents\0");
        if (changed){
            switch (shader){
                case 0:
                    material = Shader::getStandardBlinnPhong()->createMaterial();
                    break;
                case 1:
                    material = Shader::create()
                            .withSourceResource("debug_uv_vert.glsl", ShaderType::Vertex)
                            .withSourceResource("debug_uv_frag.glsl", ShaderType::Fragment)
                            .build()->createMaterial();
                    break;
                case 2:
                    material = Shader::create()
                            .withSourceResource("debug_normal_vert.glsl", ShaderType::Vertex)
                            .withSourceResource("debug_normal_frag.glsl", ShaderType::Fragment)
                            .build()->createMaterial();
                    break;
                case 3:
                    material = Shader::create()
                            .withSourceResource("debug_tangent_vert.glsl", ShaderType::Vertex)
                            .withSourceResource("debug_tangent_frag.glsl", ShaderType::Fragment)
                            .build()->createMaterial();
                    break;
                default:
                    std::cout << "Err"<<std::endl;
            }

            material->setColor({1.0f,1.0f,1.0f,1.0f});
            material->setSpecularity(Color(1,1,1,20.0f));
        }
        changed |= ImGui::Checkbox("Recompute normals",&recomputeNormals);
        changed |= ImGui::Checkbox("Recompute tangents",&recomputeTangents);
        changed |= ImGui::Combo("Primitive",&primitive,"Cube\0Sphere\0Quad\0Torus\0");
        if (changed){
            switch (primitive){
                case 0:
                    mesh = Mesh::create().withCube(1).withRecomputeNormals(recomputeNormals).withRecomputeTangents(recomputeTangents).build();
                    break;
                case 1:
                    mesh = Mesh::create().withSphere().withRecomputeNormals(recomputeNormals).withRecomputeTangents(recomputeTangents).build();
                    break;
                case 2:
                    mesh = Mesh::create().withQuad(1).withRecomputeNormals(recomputeNormals).withRecomputeTangents(recomputeTangents).build();
                    break;
                case 3:
                    mesh = Mesh::create().withTorus().withRecomputeNormals(recomputeNormals).withRecomputeTangents(recomputeTangents).build();
                    break;
                default:
                    std::cout << "Err"<<std::endl;
            }
        }

    }
private:
    bool recomputeNormals = false;
    bool recomputeTangents = false;
    int shader = 0;
    int primitive = 0;
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    int i=0;
};

int main() {
    std::make_unique<PrimitivesTest>();
    return 0;
}
