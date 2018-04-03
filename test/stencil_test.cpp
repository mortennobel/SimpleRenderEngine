#include <iostream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <sre/SDLRenderer.hpp>
#include <sre/impl/GL.hpp>
#include <sre/Inspector.hpp>
#include <sre/ModelImporter.hpp>

using namespace sre;

class StencilExample{
public:
    StencilExample(){
        r.init();

        std::vector<std::shared_ptr<Material>> materials_unused;

        mesh = sre::ModelImporter::importObj("examples_data/", "suzanne.obj", materials_unused);
        plane = Mesh::create().withQuad(1).build();

        camera.setPerspectiveProjection(45,0.1,10);
        camera.lookAt({0,0,3.5},{0,0,0},{0,1,0});

        worldLights.setAmbientLight(glm::vec3{0.05});
        worldLights.addLight(Light::create().withPointLight({0.5,2,0.5}).build());

        matStencilWrite = Shader::create()
                .withSourceFile("unlit_vert.glsl", ShaderType::Vertex)
                .withSourceFile("unlit_frag.glsl", ShaderType::Fragment)
                .withDepthWrite(false)
                .withStencil(Stencil{
                    .func = StencilFunc ::Always,
                    .ref = 1,
                    .mask = 1,
                    .fail = StencilOp::Replace,
                    .zfail = StencilOp::Replace,
                    .zpass = StencilOp::Replace,
                })
                .withName("StencilWrite").build()->createMaterial();

        matStencilTest = Shader::create()
                .withSourceFile("unlit_vert.glsl", ShaderType::Vertex)
                .withSourceFile("unlit_frag.glsl", ShaderType::Fragment)
                .withStencil(Stencil{
                        .func = StencilFunc ::Equal,
                        .ref = 1,
                        .mask = 1
                })
                .withName("StencilWrite").build()->createMaterial();
        matStencilTest->setColor({0.3,0.3,0.3});

        mat1 = Shader::getStandardPhong()->createMaterial();
        shadow = Shader::getUnlit()->createMaterial();
        shadow->setColor({0.3,0.3,0.3});

        std::string info;
        if (!mat1->getShader()->validateMesh(mesh.get(), info)){
            std::cout << info <<std::endl;
        } else {
            std::cout << "Mesh ok" << std::endl;
        }

        r.frameRender = [&](){
            render();
        };
        r.mouseEvent = [&](SDL_Event& event){
            if (event.type == SDL_MOUSEMOTION){
                float mouseSpeed = 1/50.0f;
                rotateY = event.motion.x*mouseSpeed;
                rotateX = event.motion.y*mouseSpeed;
            }
            if (event.button.button==SDL_BUTTON_RIGHT){
                showInspector = true;
            }
        };
        r.startEventLoop();
    }

    void render(){
        auto rp = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true,{0,0,0,1})
                .withClearStencil(true)
                .withWorldLights(&worldLights)
                .build();

        auto pos = worldLights.getLight(0)->position;

        float y = pos.y-0.01f-shadowPlane;
        glm::mat4 shadow = glm::transpose(glm::mat4(
                1,0,0,0,
                0,1,0,0,
                0,0,1,0,
                0,1/-y,0,0
        ));
        glm::mat4 projectedShadow = glm::translate(pos) * shadow * glm::translate(-pos);


        auto modelRotation = (glm::rotate(rotateX, glm::vec3(1,0,0))*glm::rotate(rotateY, glm::vec3(0,1,0)));

        if (drawShadow) {
            if (useStencil){
                rp.draw(plane, glm::translate(glm::vec3{0,shadowPlane,0})*glm::rotate(-glm::half_pi<float>(),glm::vec3{1,0,0}), matStencilWrite);
                rp.draw(mesh, projectedShadow * modelRotation, matStencilTest);
            } else {
               rp.draw(mesh, projectedShadow * modelRotation, this->shadow);
            }
        }
        rp.draw(mesh, modelRotation, mat1);
        if (drawPlane){
            rp.draw(plane, glm::translate(glm::vec3{0,shadowPlane,0})*glm::rotate(-glm::half_pi<float>(),glm::vec3{1,0,0}), mat1);
        }

        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowContentSize(ImVec2(300,100));
        ImGui::Begin("Shadow");
        ImGui::DragFloat("Ground height ",&shadowPlane,0.1f);
        ImGui::Checkbox("Draw plane",&drawPlane);
        ImGui::Checkbox("Draw shadow",&drawShadow);
        ImGui::Checkbox("Use stencil",&useStencil);
        ImGui::DragFloat3("Light pos",&worldLights.getLight(0)->position.x,.1f);
        ImGui::End();

        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }
    }
private:
    float time;
    SDLRenderer r;
    Camera camera;
    float shadowPlane = -1;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Mesh> plane;
    std::shared_ptr<Material> mat1;
    std::shared_ptr<Material> matStencilWrite;
    std::shared_ptr<Material> matStencilTest;
    std::shared_ptr<Material> shadow;
    WorldLights worldLights;
    float rotateX = 0;
    float rotateY = 0;
    bool drawPlane = true;
    bool drawShadow = true;
    bool useStencil = false;
    bool showInspector = false;
};

int main() {
    new StencilExample();

    return 0;
}

