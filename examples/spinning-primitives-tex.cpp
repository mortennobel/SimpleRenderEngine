#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <sre/SDLRenderer.hpp>

using namespace sre;

class SpinningPrimitivesTexExample {
public:
    SpinningPrimitivesTexExample(){
        r.init();

        camera.lookAt({0,0,6},{0,0,0},{0,1,0});
        camera.setPerspectiveProjection(60,0.1,100);
        material = Shader::getUnlit()->createMaterial();
        material->setTexture(Texture::create().withFile("examples_data/test.png").withGenerateMipmaps(true).build());
        mesh[0] = Mesh::create()
                .withQuad()
                .build();
        mesh[1] = Mesh::create()
                .withSphere()
                .build();
        mesh[2] = Mesh::create()
                .withCube()
                .build();
        mesh[3] = Mesh::create()
                .withTorus()
                .build();


        r.frameRender = [&](){
            render();
        };
        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{1, 0, 0, 1})
                .build();

        const float speed = .5f;
        int index = 0;
        for (int x=0;x<2;x++){
            for (int y=0;y<2;y++){
                glm::mat4 modelTransform = glm::translate(glm::vec3(-1.5+x*3,-1.5+y*3,0)) *
                                           glm::eulerAngleY(glm::radians( i * speed));
                renderPass.draw(mesh[index], modelTransform, material);
                index++;
            }
        }
        i++;
    }
private:
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Material> material;
    std::shared_ptr<Mesh> mesh[4];
    int i=0;
};

int main() {
    new SpinningPrimitivesTexExample();

    return 0;
}

