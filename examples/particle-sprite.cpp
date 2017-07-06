#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Camera.hpp"
#include "sre/Mesh.hpp"
#include "sre/Material.hpp"
#include "sre/Shader.hpp"
#define SDL_MAIN_HANDLED
#include "SDL.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <sre/SDLRenderer.hpp>
#include "sre/imgui_sre.hpp"

using namespace sre;

class ParticleSpriteExample{
public:
    ParticleSpriteExample(){
        r.init();
        camera = new Camera();
        camera->lookAt({0,0,3},{0,0,0},{0,1,0});
        camera->setPerspectiveProjection(60,0.1,100);
        shaderParticles = Shader::getStandardParticles();
        material = new Material(shaderParticles);
        material->setTexture(Texture::create().withFile("examples/data/t_explosionsheet.png").build());

        particleMesh = createParticles();
        r.frameUpdate = [&](float deltaTime){
            update(deltaTime);
        };
        r.frameRender = [&](Renderer* r){
            render(r);
        };
        r.startEventLoop();
    }

    void update(float deltaTime){
        timeF += deltaTime;
        if (ortho) {
            camera->setOrthographicProjection(-4,4,-4,4,-4,100);
        } else {
            camera->setPerspectiveProjection(60,0.1,10);
        }
        updateParticles(particleMesh, spriteUV, uvSize, uvRotation, size);
    }

    void render(Renderer* r){

        auto rp = r->createRenderPass()
                .withCamera(*camera)
                .withClearColor(true,{1,0,0.0,1})
                .build();

        rp.draw(particleMesh, glm::mat4(1), material);

        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
        {
            ImGui::Text("Particle sprite");
            ImGui::Checkbox("Orthographic proj",&ortho);
            ImGui::Checkbox("Sprite animation",&spriteAnimation);
            if (spriteAnimation) {
                updateParticlesAnimation(timeF, spriteUV,uvSize, uvRotation);
            }
            ImGui::SliderFloat("size", &size, 0.0f, 200.0f);
            ImGui::DragFloat2("uv", &(spriteUV.x), 0.1f);
            ImGui::SliderFloat("uv", &uvSize, 0.0f, 1.0f);
            ImGui::SliderFloat("uvRotation", &uvRotation, 0.0f, 2*3.1415f);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
    }

    std::shared_ptr<Mesh> createParticles(){
        std::vector<glm::vec3> positions;
        std::vector<glm::vec4> colors;
        std::vector<glm::vec4> uvs;
        std::vector<float> sizes;

        positions.push_back({0,0,0});
        colors.push_back({1,1,1,1});
        sizes.push_back(10.0f);

        return Mesh::create()
                .withPositions(positions)
                .withParticleSizes(sizes)
                .withColors(colors)
                .withUVs(uvs)
                .withMeshTopology(MeshTopology::Points)
                .build();
    }

    void updateParticlesAnimation(float time, glm::vec2& pos,float& size, float& rotation){
        int frame = ((int)(time*10))%16;
        int frameX = 3-frame%4;
        int frameY = frame/4;
        pos = glm::vec2(frameX * 0.25f, frameY * 0.25f);
        size = 0.25f;
        rotation = 0;
    }

    void updateParticles(std::shared_ptr<Mesh> particleMesh, glm::vec2 uv, float uvSize, float rotation, float size){
        std::vector<glm::vec3> positions;
        std::vector<glm::vec4> uvs;
        std::vector<float> sizes;

        positions.push_back({0,0,0});
        uvs.push_back({uv,uvSize,rotation});
        sizes.push_back(size);

        particleMesh->update()
                .withPositions(positions)
                .withUVs(uvs)
                .withParticleSizes(sizes)
                .build();
    }
private:
    SDLRenderer r;
    glm::vec2 spriteUV = glm::vec2(0, 0);
    bool spriteAnimation = false;
    bool ortho = false;
    float uvSize = 1.0;
    float uvRotation = 0.0;
    float size = 200.0f;
    float timeF = 0;
    std::shared_ptr<Mesh> particleMesh;
    std::shared_ptr<Shader> shaderParticles;
    Material* material;
    Camera* camera;
};

int main() {
    new ParticleSpriteExample();

    return 0;
}