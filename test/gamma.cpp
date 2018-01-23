#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>

using namespace sre;

class GammaTest{
public:
    GammaTest(){
        r.init();

        mesh = Mesh::create().withCube()
                .build();

        mat1 = Shader::getUnlit()->createMaterial();
        mat1->setTexture(Texture::create().withFile("test_data/gamma-test.png").build());

        r.frameRender = [&](){
            render();
        };
        r.startEventLoop();
    }

    void render(){
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{1,0,0,1})
                .build();

        rp.draw(mesh, glm::mat4(1), mat1);

    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> mat1;
};

int main() {
    new GammaTest();

    return 0;
}

