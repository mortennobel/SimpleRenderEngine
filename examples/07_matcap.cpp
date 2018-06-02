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
#include <sre/ModelImporter.hpp>

using namespace sre;

class MapcapExample{
public:
    MapcapExample(){
        r.init();

        std::vector<std::shared_ptr<Material>> materials_unused;

        mesh = sre::ModelImporter::importObj("examples_data/", "suzanne.obj", materials_unused);

        camera.setPerspectiveProjection(45,0.1,10);
        camera.lookAt({0,0,3.5},{0,0,0},{0,1,0});

        for (int i=0;i<4;i++){
            textures[i] = sre::Texture::create().withFile(std::string("examples_data/matcap_0000")+std::to_string(i+1)+".png").build();
        }
        std::string vertexShaderSource =  R"(#version 330
in vec4 position;
in vec3 normal;
out vec3 vNormal;

#pragma include "global_uniforms_incl.glsl"

void main(void) {
    gl_Position = g_projection * g_view * g_model * position;
    vNormal = normalize(mat3(g_view) *g_model_it * normal);
}
)";
        std::string fragmentShaderSource = R"(#version 330
out vec4 fragColor;
in vec3 vNormal;

uniform sampler2D tex;

void main(void)
{
    vec3 normal = normalize(vNormal);
    fragColor = texture(tex,normal.xy * 0.5 + 0.5);
}
)";

        mat1 = Shader::create()
                .withSourceString(vertexShaderSource,ShaderType::Vertex)
                .withSourceString(fragmentShaderSource, ShaderType::Fragment)
                .build()->createMaterial();
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
                .build();

        mat1->setTexture(textures[texture]);
        rp.draw(mesh, glm::rotate(rotateX, glm::vec3(1,0,0))*glm::rotate(rotateY, glm::vec3(0,1,0)), mat1);

        const char* items[] = { "00001", "00002", "00003", "00004"};
        ImGui::SetNextWindowPos(ImVec2(0,0));
        ImGui::SetNextWindowContentSize(ImVec2(300,100));
        ImGui::Begin("Matcap");
        ImGui::ListBox("Texture",&texture, items,4);
        ImGui::End();

        static Inspector inspector;
        inspector.update();
        if (showInspector){
            inspector.gui();
        }
    }
private:
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> mat1;
    std::array<std::shared_ptr<sre::Texture>,4> textures;
    float rotateX = 0;
    float rotateY = 0;
    int texture = 0;
    bool showInspector = false;
};

int main() {
    std::make_unique<MapcapExample>();
    return 0;
}

