#include <iostream>
#include <vector>
#include <fstream>

#include "sre/Texture.hpp"
#include "sre/Renderer.hpp"
#include "sre/Inspector.hpp"
#include "sre/Material.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "sre/SDLRenderer.hpp"

using namespace sre;

class GUIHighDPI {
public:
    GUIHighDPI(bool highDPI)
            :r{}
    {
        r.init()
            .withSdlWindowFlags( (highDPI ? SDL_WINDOW_ALLOW_HIGHDPI: 0) | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

        r.setWindowTitle(highDPI ? "High DPI": "Normal DPI");

        // connect render callback
        r.frameRender = [&](){
            frameRender();
        };
        r.mouseEvent = [&](SDL_Event& e) {
            if (e.type == SDL_MOUSEMOTION) {
                mousePos.x = e.motion.x;
                mousePos.y = e.motion.y;
            }
        };
        // start render loop
        r.startEventLoop();
    }


    void frameRender(){
        RenderPass rp = RenderPass::create()
                .build();
        ImGuiCond cond = ImGuiCond_Always;
        ImVec2 pos{100,100};
        ImGui::SetNextWindowPos(pos, cond);
        ImGui::Begin("Window");
        ImGui::Text("Mouse position %i %i",mousePos.x, mousePos.y);


        int index = SDL_GetWindowDisplayIndex(r.getSDLWindow());
        ImGui::Text("Current Display %i",index);

        auto& io = ImGui::GetIO();

        ImGui::Text("ImGui mousePos %.1f %.1f", io.MousePos.x, io.MousePos.y);

        glm::ivec2 winSize;
        SDL_GetWindowSize(r.getSDLWindow(),
                          &winSize.x,
                          &winSize.y);
        ImGui::Text("Windows size %i %i",winSize.x,winSize.y);

        SDL_GL_GetDrawableSize(r.getSDLWindow(),
                          &winSize.x,
                          &winSize.y);
        ImGui::Text("Drawable size %i %i",winSize.x,winSize.y);


        int displays = SDL_GetNumVideoDisplays();

        for (int i=0;i<displays;i++){
            ImGui::Text("Display %i",i);
            float ddpi;
            float hdpi;
            float vdpi;
            int res = SDL_GetDisplayDPI(i,
                    &ddpi,
                    &hdpi,
                    &vdpi);
            if (res == 0){
                ImGui::Text("DPI ddpi %f hdpi %f vdpi %f", ddpi, hdpi, vdpi);
            } else {
                ImGui::Text("SDL_GetDisplayDPI not supported");
            }
        }
        ImGui::End();
        ImGui::Begin("Widgets");
        ImGui::Button("Press me");
        static bool checked = false;
        ImGui::Checkbox("Click on me", &checked );
        ImGui::End();
    }
private:
    SDLRenderer r;
    glm::ivec2 mousePos;
};

int main() {
    std::make_unique<GUIHighDPI>(true);
    std::make_unique<GUIHighDPI>(false);
    return 0;
}

