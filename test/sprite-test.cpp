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
#include <sre/Inspector.hpp>


using namespace sre;

class SpriteExample {
public:
    SpriteExample()
    {
        r.init()
                .withSdlInitFlags(SDL_INIT_EVERYTHING)
                .withSdlWindowFlags(SDL_WINDOW_OPENGL);

        camera.setWindowCoordinates();

        // Create sprite atlas using default texture properties
        atlas = SpriteAtlas::create("test_data/sprite_test.json","test_data/sprite_test.png");


        // create sprite atlas using point sampling and no mipmaps
        auto tex = Texture::create().withFile("test_data/sprite_test.png").withFilterSampling(false).withGenerateMipmaps(false).build();
        atlas2 = SpriteAtlas::create("test_data/sprite_test.json",tex);

        atlas3 = SpriteAtlas::create("test_data/circle-slices-cropped.json","test_data/circle-slices-cropped.png");

        circle = Mesh::create().withSphere(32,64,512/2).build();

        quad = Mesh::create().withName("Tex mesh") .withPositions({{0,0,0}}).withUVs({{0,0,0,0}}).build();
        material = Shader::create()
                .withSourceFile("unlit_vert.glsl", ShaderType::Vertex)
                .withSourceFile("unlit_frag.glsl", ShaderType::Fragment)
                .withBlend(BlendType::AlphaBlending)
                .build()->createMaterial();
        material2 = Shader::getUnlit()->createMaterial();
        updateQuad(quad, atlas->getTexture());

        r.frameRender = [&](){
            render();
        };

        r.startEventLoop();
    }

    void updateQuad(std::shared_ptr<Mesh> mesh, std::shared_ptr<Texture> tex){
        int w = tex->getWidth();
        int h = tex->getHeight();
        mesh->update()
                .withPositions({{0,0,0},{w/2,0,0},{w/2,h/2,0},  {0,0,0}, {w/2,h/2,0}, {0,h/2,0}})
                .withUVs({{0,0,0,0},{1,0,0,0},{1,1,0,0},  {0,0,0,0}, {1,1,0,0}, {0,1,0,0}})
                .build();
        material->setTexture(tex);
    }

    void render(){
        auto renderPass = RenderPass::create()
                .withCamera(camera)
                .withClearColor(true, {.3f, .3f, 1, 1})
                .build();



        static int selectedAtlas = 0;
        const char * atlasNames = "Sprite\0Sprite - nofilter\0circle\0";
        if (ImGui::Combo("Atlas", &selectedAtlas,atlasNames)){
            auto atlasPtr = selectedAtlas==0?atlas:(selectedAtlas==1?atlas2:atlas3);
            updateQuad(quad, atlasPtr->getTexture());
        }

        auto atlasPtr = selectedAtlas==0?atlas:(selectedAtlas==1?atlas2:atlas3);

        auto names = atlasPtr->getNames();

        const auto ** namesPtr = new const char*[names.size()];
        for (int i=0;i<names.size();i++){
            namesPtr[i] = names[i].c_str();
        }

        static int selectedSprite = 0;
        selectedSprite = selectedSprite % (int)names.size();
        ImGui::Combo("Sprite", &selectedSprite,namesPtr,(int)names.size());
        delete namesPtr;

        auto sprite = atlasPtr->get(names.at(selectedSprite));
        static glm::vec4 color (1,1,1,1);
        static glm::vec2 scale(1,1);
        static glm::vec2 position(300,300);
        static float rotation = 0;
        static glm::bvec2 flip = {false,false};
        static bool inspectorEnabled = false;

        ImGui::ColorEdit4("Color", &color.x,ImGuiColorEditFlags_RGB|ImGuiColorEditFlags_Float);
        ImGui::DragFloat2("Pos", &position.x,1);
        ImGui::DragFloat("Rotation", &rotation,1);
        ImGui::DragFloat2("Scale", &scale.x,0.1);
        ImGui::Checkbox("Flip x", &flip.x);
        ImGui::Checkbox("Flip y", &flip.y);
        ImGui::Checkbox("Inspector", &inspectorEnabled);
        ImGui::DragInt("sprite1 OrderInBatch", &spriteIndex1,1,0,10);
        ImGui::DragInt("sprite2 OrderInBatch", &spriteIndex2,1,0,10);
        ImGui::Checkbox("useSameAtlas", &useSameAtlas);
        ImGui::Checkbox("useAddSprites", &useAddSprites);

        sprite.setColor(color);
        sprite.setScale(scale);
        sprite.setPosition(position);
        sprite.setRotation(rotation);
        sprite.setFlip(flip);
        sprite.setOrderInBatch(spriteIndex1);



        renderPass.draw(circle,glm::translate(glm::vec3(position.x,position.y,0))*glm::scale(glm::vec3(1,1,1/1000.0)), material2);
        //renderPass.draw(quad,glm::mat4(1), material);
        std::vector<Sprite> sprites{{sprite}};
        auto sb = useAddSprites?
                  SpriteBatch::create()
                          .addSprites(sprites.begin(), sprites.end())
                          .build()
                               :
                  SpriteBatch::create()
                .addSprite(sprite)
                .build();

        renderPass.draw(sb);

        std::vector<glm::vec3> lines;
        auto spriteCorners = sprite.getTrimmedCorners();
        for (int i=0;i<4;i++){
            lines.emplace_back(spriteCorners[i],0);
            lines.emplace_back(spriteCorners[(i+1)%4],0);
        }
        renderPass.drawLines(lines);

        static Inspector inspector;
        inspector.update();
        if (inspectorEnabled){
            inspector.gui();
        }
    }
private:
    std::shared_ptr<SpriteAtlas> atlas;
    std::shared_ptr<SpriteAtlas> atlas2;
    std::shared_ptr<SpriteAtlas> atlas3;
    std::shared_ptr<Mesh> quad;
    std::shared_ptr<Material> material;
    std::shared_ptr<Material> material2;
    int spriteIndex1 = 0;
    int spriteIndex2 = 0;
    SDLRenderer r;
    Camera camera;
    bool useSameAtlas = false;
    bool useAddSprites = false;
    std::shared_ptr<SpriteBatch> world;

    std::shared_ptr<Mesh> circle;
};

int main() {
    new SpriteExample();
    return 0;
}
