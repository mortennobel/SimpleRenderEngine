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
#include <sre/Inspector.hpp>

constexpr int BOX_GRID_DIM = 40;
constexpr int BENCHMARK_SIZE = 40;

using namespace sre;

float renderTimeGetter(void* data, int offset){
    float* floatData = static_cast<float *>(data);
    int id = (int)cbrtf((float)offset);
    return floatData[id];
}
//
class Benchmark64KExampleHeavy {
public:
    Benchmark64KExampleHeavy() {
        r.init();

        camera.setPerspectiveProjection(60,0.1,100);

        worldLights.addLight(Light::create().withDirectionalLight(glm::vec3(1,1,1)).withColor(Color(1,1,1),1).build());
        renderTime.resize(BOX_GRID_DIM+1,0);
        stateChanges.resize(BOX_GRID_DIM+1,0);
        drawCalls.resize(BOX_GRID_DIM+1,0);
        meshes = {
                Mesh::create().withCube(0.25f).build(),
                Mesh::create().withSphere().build(),
                Mesh::create().withTorus().build(),
                Mesh::create().withQuad().build(),
                Mesh::create().withCube(0.35f).build(),
        };


        materials = {
                Shader::getUnlit()->createMaterial(),
                Shader::getStandardBlinnPhong()->createMaterial(),
                Shader::getStandardPBR()->createMaterial(),
                Shader::getUnlit()->createMaterial(),
                Shader::getStandardBlinnPhong()->createMaterial(),
                Shader::getStandardPBR()->createMaterial(),
                Shader::getUnlit()->createMaterial(),
        };

        for (int i = 0;i<materials.size();i++){
            // explicit use
            if (i % 2==0){
                materials[i]->setTexture(Texture::create().withFile("test_data/t_explosionsheet.png").withGenerateMipmaps(true).build());
            } else {
                materials[i]->setTexture(Texture::create().withFile("test_data/gamma-test.png").withGenerateMipmaps(true).build());
            }
        }

        int boxI = 0;
        float offset = -(gridSize / 2.0f);
        for (int i = 0; i < BOX_GRID_DIM; ++i) {
            for (int j = 0; j < BOX_GRID_DIM; ++j) {
                for (int k = 0; k < BOX_GRID_DIM; ++k) {
                    box[i][j][k] = {
                            glm::rotate((float) (boxI / M_PI), glm::vec3(1,1,1)),
                            glm::translate(glm::vec3(i + offset, j + offset, k + offset))
                    };
                    boxI++;
                }
            }
        }
        r.frameUpdate = [&](float delta){
            update(delta);
        };
        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }
    void update(float delta){
        static float totalTime = 0;
        eyeRotation += 0.002;
        eyePosition[0] = (float) (sin(eyeRotation) * eyeRadius);
        eyePosition[2] = (float) (cos(eyeRotation) * eyeRadius);
        if (cameraInCenter){
            camera.lookAt( {0, 0, 0}, glm::normalize(eyePosition),{0, 1, 0});
        } else {
            camera.lookAt(eyePosition, {0, 0, 0}, {0, 1, 0});
        }
        totalTime += delta;
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    auto & boxRef = box[i][j][k];
                    // update rotation
                    boxRef.rotationMatrix = glm::rotate(glm::mat4(1),0.02f*totalTime, glm::vec3(0,1,0));
                    modelMatrix[i][j][k] = boxRef.translationMatrix * boxRef.rotationMatrix;
                }
            }
        }
    }

    void render(){
        if (benchmarkCount >=0){
            if (benchmarkCount>0 && benchmarkCount-1 < renderTime.size()){
                //Renderer::instance->getRenderStats().drawCalls;
                renderTime[benchmarkCount-1] = SDLRenderer::instance->getLastFrameStats().z;
                stateChanges[benchmarkCount-1] = Renderer::instance->getRenderStats().stateChangesMaterial + Renderer::instance->getRenderStats().stateChangesShader + Renderer::instance->getRenderStats().stateChangesMesh;
                drawCalls[benchmarkCount-1] = Renderer::instance->getRenderStats().drawCalls;
            }
            if (benchmarkCount+1 == BOX_GRID_DIM){
                benchmarkCount = -1;
                gridSize = BOX_GRID_DIM/3;
            } else {
                benchmarkCount++;
                gridSize = benchmarkCount;
            }

        }
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withWorldLights(&worldLights)
                .withClearColor(true, {0, 0, 0, 1})
                .build();
        int id=0;
        for (int i = 0; i < gridSize; ++i) {
            for (int j = 0; j < gridSize; ++j) {
                for (int k = 0; k < gridSize; ++k) {
                    renderPass.draw(meshes[id%meshes.size()], modelMatrix[i][j][k], materials[id%materials.size()]);
                    id++;
                }
            }
        }
        i++;
        static Inspector inspector;
        inspector.update();

        bool gridChanged = ImGui::SliderInt("Grid size",&gridSize,1,BOX_GRID_DIM);
        if (gridChanged){
            float offset = -(gridSize / 2.0f);
            for (int i = 0; i < gridSize; ++i) {
                for (int j = 0; j < gridSize; ++j) {
                    for (int k = 0; k < gridSize; ++k) {
                        auto & boxRef = box[i][j][k];
                        // update rotation
                        boxRef.translationMatrix = glm::translate(glm::vec3(i + offset, j + offset, k + offset));
                    }
                }
            }

        }
        ImGui::Checkbox("Camera in center",&cameraInCenter);

        if (benchmarkCount >= 0){
            ImGui::LabelText("","Benchmark running");
        } else {
            if (ImGui::Button("Start benchmark")){
                benchmarkCount = 1;
                for (auto & v : renderTime){
                    v = 0;
                }
                for (auto &v : stateChanges){
                    v = 0;
                }
                for (auto &v : drawCalls){
                    v = 0;
                }

            }
        }


        ImGui::PlotLines("Objects/Render time",&renderTimeGetter,renderTime.data(),(BENCHMARK_SIZE-1)*(BENCHMARK_SIZE-1)*(BENCHMARK_SIZE-1), 0, "Render time", FLT_MAX,FLT_MAX,ImVec2(ImGui::CalcItemWidth(),150));
        ImGui::PlotLines("Objects/State changes",&renderTimeGetter,stateChanges.data(),(BENCHMARK_SIZE-1)*(BENCHMARK_SIZE-1)*(BENCHMARK_SIZE-1), 0, "State changes", FLT_MAX,FLT_MAX,ImVec2(ImGui::CalcItemWidth(),150));
        ImGui::PlotLines("Objects/Draw calls",&renderTimeGetter,drawCalls.data(),(BENCHMARK_SIZE-1)*(BENCHMARK_SIZE-1)*(BENCHMARK_SIZE-1), 0, "Draw calls", FLT_MAX,FLT_MAX,ImVec2(ImGui::CalcItemWidth(),150));
        inspector.gui();

    }
private:
    int gridSize = BOX_GRID_DIM/2;
    float eyeRadius = 30;
    float eyeRotation = 0;
    glm::vec3 eyePosition = {0, eyeRadius, 0};
    bool cameraInCenter = false;
    SDLRenderer r;
    Camera camera;
    WorldLights worldLights;
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::vector<std::shared_ptr<Material>> materials;
    std::vector<float> renderTime;
    std::vector<float> stateChanges;
    std::vector<float> drawCalls;
    int benchmarkCount = -1;
    int i=0;
    struct Box{
        glm::mat4 rotationMatrix;
        glm::mat4 translationMatrix;
    } box[BOX_GRID_DIM][BOX_GRID_DIM][BOX_GRID_DIM];
    glm::mat4 modelMatrix[BOX_GRID_DIM][BOX_GRID_DIM][BOX_GRID_DIM];

};

int main() {
    std::make_unique<Benchmark64KExampleHeavy>();
    return 0;
}
