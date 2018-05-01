#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>

using namespace sre;

class GammaTest{
public:
    GammaTest(){
        r.init();

        mesh = Mesh::create().withQuad(0.5f)
                .build();

        camera.setWindowCoordinates();

        mat1 = Shader::getUnlit()->createMaterial();
        tex = Texture::create().withFile("test_data/gamma-test.png")
                .withFilterSampling(false)
                .build();
        tex2 = Texture::create().withFile("test_data/gamma-small.png")
                .withFilterSampling(false)
                .withDumpDebug()
                .build();
        mat1->setTexture(tex2);

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

        rp.draw(mesh,
                glm::translate(glm::vec3{(int)(tex->getWidth()*0.5),(int)(tex->getHeight()*0.5),0})*
                        glm::scale(glm::vec3(tex->getWidth(), tex->getHeight(),1)), mat1);



    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> mat1;
    std::shared_ptr<Texture> tex;
    std::shared_ptr<Texture> tex2;
};

int main() {
    GammaTest gammaTest;

    return 0;
}

