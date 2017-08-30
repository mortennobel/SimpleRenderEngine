//
// Created by Morten Nobel-Jørgensen on 18/07/2017.
//

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
#include <glm/gtx/string_cast.hpp>
#include <sre/SpriteAtlas.hpp>
#include <sre/Profiler.hpp>


using namespace sre;

class SpriteExample {
public:
    SpriteExample()
    :r{}
    {
        r.init();

        atlas = SpriteAtlas::create("examples_data/PlanetCute.json","examples_data/PlanetCute.png");

        camera.setWindowCoordinates();

        r.frameRender = [&](){
            render();

        };

        r.startEventLoop();
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true, {.3, .3, 1, 1})
                .build();


        auto names = atlas->getNames();
        static bool demoWorld = false;
        ImGui::Checkbox("RenderWorld", &demoWorld );
        if (demoWorld ){
            if (world == nullptr){
                auto worldBuilder = SpriteBatch::create();

                glm::ivec2 blockSize(101,50);

                std::vector<std::vector<int>> fields =
                        {{0,0,10,10,9,9,10,0,0,10,10,9,9,10},
                         {0,0,10,10,9,9,10,0,0,10,10,9,9,10},
                         {0,0,10,10,-1,-1,-1,0,0,10,10,-1,-1,-1},
                        };
                for (int y=0;y<fields.size();y++){
                    for (int x=0;x<fields[y].size();x++) {
                        int id = fields[y][x];
                        if (id==-1) continue;

                        auto sprite = atlas->get(names.at(id));
                        sprite.setPosition(glm::vec2(x*blockSize.x,y*blockSize.y));
                        worldBuilder.addSprite(sprite);
                    }
                }
                world = worldBuilder.build();
            }
            static float d=0;
            renderPass.draw(world, glm::translate(glm::vec3(-500+sin(d)*500,0,0)));
            d+=0.016;
        } else {
            world.reset();
            static const char** namesPtr = new const char*[names.size()];
            for (int i=0;i<names.size();i++){
                namesPtr[i] = names[i].c_str();
            }
            static int selected = 0;

            ImGui::Combo("Sprite", &selected,namesPtr,names.size());

            auto sprite = atlas->get(names.at(selected));
            static glm::vec4 color (1,1,1,1);
            static glm::vec2 scale(1,1);
            static glm::vec2 position(200,100);
            static float rotation = 0;
            static glm::bvec2 flip = {false,false};

            ImGui::ColorEdit4("Color", &color.x,ImGuiColorEditFlags_RGB|ImGuiColorEditFlags_Float);
            ImGui::DragFloat2("Pos", &position.x,1);
            ImGui::DragFloat("Rotation", &rotation,1);
            ImGui::DragFloat2("Scale", &scale.x,0.1);
            ImGui::Checkbox("Flip x", &flip.x);
            ImGui::Checkbox("Flip y", &flip.y);

            sprite.setColor(color);
            sprite.setScale(scale);
            sprite.setPosition(position);
            sprite.setRotation(rotation);
            sprite.setFlip(flip);

            auto sb = SpriteBatch::create()
                    .addSprite(sprite).build();
            renderPass.draw(sb);

            std::vector<glm::vec3> lines;
            auto spriteCorners = sprite.getCorners();
            for (int i=0;i<4;i++){
                lines.push_back({spriteCorners[i],0});
                lines.push_back({spriteCorners[(i+1)%4],0});
            }
            renderPass.drawLines(lines);
        }

        auto& renderStats = Renderer::instance->getRenderStats();

        float bytesToMB = 1.0f/(1024*1024);
        ImGui::Text("sre draw-calls %i meshes %i (%.2fMB) textures %i (%.2fMB) shaders %i", renderStats.drawCalls,renderStats.meshCount, renderStats.meshBytes*bytesToMB, renderStats.textureCount, renderStats.textureBytes*bytesToMB, renderStats.shaderCount);
    }
private:
    std::shared_ptr<SpriteAtlas> atlas;
    SDLRenderer r;
    Camera camera;
    std::shared_ptr<SpriteBatch> world;
};

int main() {
    new SpriteExample();
    return 0;
}
