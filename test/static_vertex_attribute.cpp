#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"
#include "sre/impl/GL.hpp"


#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>


using namespace sre;

class StaticVertexAttribute {
public:
    StaticVertexAttribute(){
        r.init();


        material = Shader::getUnlit()->createMaterial();

        std::vector<glm::vec3> pos {
                {0,0,0},
                {1,0,0},
                {0,1,0}
        };

        mesh = Mesh::create().withPositions(pos).build();

        pos = {
                {0,0,0},
                {1,0,0},
                {1,0,0},
                {0,-1,0},
                {0,-1,0},
                {0,0,0},
        };

        meshWire = Mesh::create()
                .withPositions(pos)
                .withMeshTopology(MeshTopology::Lines)
                .build();

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
        renderPass.draw(mesh, glm::eulerAngleY(glm::radians((float)i)), material);

        std::vector<glm::vec3> verts = {
                {0,0,0},
                {1,0,0},
                {1,0,0},
                {0,-1,0},
                {0,-1,0},
                {0,0,0},
        };

        // Keep a shared mesh and material
        static auto meshTopology = MeshTopology::Lines;
        static auto material = Shader::getUnlit()->createMaterial();
        std::shared_ptr<Mesh> mesh = Mesh::create()
                .withPositions(verts)
                .withMeshTopology(meshTopology)
                .build();

        // update shared mesh
        mesh->update().withPositions(verts).build();

        // update material
        material->setColor({0,1,0,1});
        renderPass.draw(mesh, glm::mat4(1), material);

        float offset = -0.2f;
        verts = {
                {0, 0+offset,0},
                {1, 0+offset,0},
                {1, 0+offset,0},
                {0,-1+offset,0},
                {0,-1+offset,0},
                {0, 0+offset,0},
        };
        // update shared mesh
        mesh->update().withPositions(verts).build();

        // update material
        material->setColor({0,1,0,1});
        renderPass.draw(mesh, glm::mat4(1), material);


        i++;
    }
private:
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> meshWire;
    std::shared_ptr<Material> material;
    int i=0;
};

int main() {
    new StaticVertexAttribute();
    return 0;
}
