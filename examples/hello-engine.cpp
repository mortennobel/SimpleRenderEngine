#include <iostream>
#include <vector>

#include "sre/SDLRenderer.hpp"

using namespace sre;
using namespace std;

int main() {
    SDLRenderer r;
    r.init();
    r.frameRender = [](Renderer* renderer){
        RenderPass rp = renderer->createRenderPass().build();
        rp.drawLines({{-.5,-.5,0},{.5,.5,0},{-.5,.5,0},{.5,-.5,0}});
    };
    r.startEventLoop();

    return 0;
}