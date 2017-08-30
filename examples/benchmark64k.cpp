#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Material.hpp"
#include "sre/SDLRenderer.hpp"

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

const int BOX_GRID_DIM = 40;

using namespace sre;

//
class Benchmark64KExample {
public:
    Benchmark64KExample() {
        r.init();

        camera = new Camera();
        camera->setPerspectiveProjection(90,0.1,100);

        material = Shader::getUnlit()->createMaterial();
        material->setTexture(Texture::create().withFile("examples_data/test.png").withGenerateMipmaps(true).build());

        mesh = Mesh::create().withCube(0.25f).build();

        int boxI = 0;
        int offset = (int) -floor(BOX_GRID_DIM / 2);
        for (int i = 0; i < BOX_GRID_DIM; ++i) {
            for (int j = 0; j < BOX_GRID_DIM; ++j) {
                for (int k = 0; k < BOX_GRID_DIM; ++k) {
                    box[i][j][k] = {
                            (float) (boxI / M_PI),
                            glm::rotate((float) (boxI / M_PI), glm::vec3(1,1,1)),
                            glm::translate(glm::vec3(i + offset, j + offset, k + offset))
                    };
                    boxI++;
                }
            }
        }
        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void render(){
        eyeRotation += 0.002;
        eyePosition[0] = (float) (sin(eyeRotation) * eyeRadius);
        eyePosition[2] = (float) (cos(eyeRotation) * eyeRadius);

        camera->lookAt(eyePosition, {0, -5, 0}, {0, 1, 0});

        auto renderPass = RenderPass::create()
                .withCamera(*camera)
                .withClearColor(true, {0, 0, 0, 1})
                .build();
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    auto & boxRef = box[i][j][k];
                    // update rotation
                    boxRef.rotationMatrix = glm::rotate(boxRef.rotationMatrix,0.02f, glm::vec3(1,1,1));
                    modelMatrix[i][j][k] = boxRef.translationMatrix * boxRef.rotationMatrix;
                    renderPass.draw(mesh, modelMatrix[i][j][k], material);
                }
            }
        }
        i++;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        auto renderStats = Renderer::instance->getRenderStats();
        float bytesToMB = 1.0f/(1024*1024);
        ImGui::Text("sre draw-calls %i meshes %i (%.2fMB) textures %i (%.2fMB) shaders %i", renderStats.drawCalls,renderStats.meshCount, renderStats.meshBytes*bytesToMB, renderStats.textureCount, renderStats.textureBytes*bytesToMB, renderStats.shaderCount);
        ImGui::SliderInt("Grid size",&gridSize,1,BOX_GRID_DIM);
    }
private:
    int gridSize = BOX_GRID_DIM/2;
    float eyeRadius = 30;
    float eyeRotation = 0;
    glm::vec3 eyePosition = {0, eyeRadius, 0};
    SDLRenderer r;
    Camera *camera;
    std::shared_ptr<Mesh> mesh;
    std::shared_ptr<Material> material;
    int i=0;
    struct Box{
        float rotate;
        glm::mat4 rotationMatrix;
        glm::mat4 translationMatrix;
    } box[BOX_GRID_DIM][BOX_GRID_DIM][BOX_GRID_DIM];
    glm::mat4 modelMatrix[BOX_GRID_DIM][BOX_GRID_DIM][BOX_GRID_DIM];

};

int main() {
    new Benchmark64KExample();
    return 0;
}
