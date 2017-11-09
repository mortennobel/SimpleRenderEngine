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
    {
        r.init();

        camera.setWindowCoordinates();

        auto tex = Texture::create().withFile("test_data/platformer.png").build();



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


        static const char** namesPtr = new const char*[names.size()];
        for (int i=0;i<names.size();i++){
            namesPtr[i] = names[i].c_str();
        }
        static int selected = 0;

        ImGui::Combo("Sprite", &selected,namesPtr,names.size());

        auto sprite = atlas->get(names.at(selected));
        static glm::vec4 color (1,1,1,1);
        static glm::vec2 scale(1,1);
        static glm::vec2 position(300,300);
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
