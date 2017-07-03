//
// Created by Morten Nobel-Jørgensen on 30/06/2017.
//

#include <chrono>
#include <iostream>
#include <sre/imgui_sre.hpp>
#include "sre/SDLRenderer.hpp"
#define SDL_MAIN_HANDLED

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif


namespace sre{

#ifdef EMSCRIPTEN
    static SDLRenderer* sdlInstance = nullptr;

    struct SDLRendererInternal{
        static void update(float f){
            sdlInstance->frame(f);
        }
    };

    void update(){
        typedef std::chrono::high_resolution_clock Clock;
        using FpSeconds = std::chrono::duration<float, std::chrono::seconds::period>;
        static auto lastTick = Clock::now();
        auto tick = Clock::now();
        float deltaTime = std::chrono::duration_cast<FpSeconds>(tick - lastTick).count();
        lastTick = tick;
        SDLRendererInternal::update(deltaTime);
    }
#endif

    SDLRenderer::SDLRenderer()
    :frameUpdate ([](float){}),
     frameRender ([](Renderer*){}),
     keyEvent ([](SDL_Event&){}),
     mouseEvent ([](SDL_Event&){}),
     controllerEvent ([](SDL_Event&){}),
     joystickEvent ([](SDL_Event&){}),
     touchEvent ([](SDL_Event&){}),
     otherEvent([](SDL_Event&){}),
     windowTitle( std::string("SimpleRenderEngine ")+std::to_string(Renderer::sre_version_major)+"."+std::to_string(Renderer::sre_version_minor )+"."+std::to_string(Renderer::sre_version_point))
    {
#ifdef EMSCRIPTEN
        sdlInstance = this;
#endif
    }

    SDLRenderer::~SDLRenderer() {
        delete r;
        r = nullptr;
#ifdef EMSCRIPTEN
        sdlInstance = nullptr;
#endif
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void SDLRenderer::frame(float deltaTimeSec){
        SDL_Event e;
        //Handle events on queue
        while( SDL_PollEvent( &e ) != 0 )
        {
            ImGui_SRE_ProcessEvent(&e);
            switch (e.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    keyEvent(e);
                    break;
                case SDL_MOUSEMOTION:
                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                case SDL_MOUSEWHEEL:
                    mouseEvent(e);
                    break;
                case SDL_CONTROLLERAXISMOTION:
                case SDL_CONTROLLERBUTTONDOWN:
                case SDL_CONTROLLERBUTTONUP:
                case SDL_CONTROLLERDEVICEADDED:
                case SDL_CONTROLLERDEVICEREMOVED:
                case SDL_CONTROLLERDEVICEREMAPPED:
                    controllerEvent(e);
                    break;
                case SDL_JOYAXISMOTION:
                case SDL_JOYBALLMOTION:
                case SDL_JOYHATMOTION:
                case SDL_JOYBUTTONDOWN:
                case SDL_JOYBUTTONUP:
                case SDL_JOYDEVICEADDED:
                case SDL_JOYDEVICEREMOVED:
                    joystickEvent(e);
                    break;
                case SDL_FINGERDOWN:
                case SDL_FINGERUP:
                case SDL_FINGERMOTION:
                    touchEvent(e);
                    break;
                default:
                    otherEvent(e);
                    break;
            }
        }

        frameUpdate(deltaTimeSec);
        frameRender(r);

        r->swapWindow();
    }

    void SDLRenderer::init(uint32_t sdlInitFlag) {
        if (running){
            return;
        }
        if (!window){
#ifdef EMSCRIPTEN
            SDL_Renderer *renderer = NULL;
            SDL_CreateWindowAndRenderer(windowWidth, windowHeight, SDL_WINDOW_OPENGL, &window, &renderer);
#else
            SDL_Init( sdlInitFlag  );
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            window = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight,SDL_WINDOW_OPENGL);
#endif
            r = new Renderer(window);
        }
    }

    void SDLRenderer::startEventLoop() {
        if (!window){
            throw std::runtime_error("SDLRenderer::init() not called");
        }

        running = true;
#ifdef EMSCRIPTEN
        emscripten_set_main_loop(update, 0, 1);
#else
        typedef std::chrono::high_resolution_clock Clock;
        using FpSeconds = std::chrono::duration<float, std::chrono::seconds::period>;
        auto lastTick = Clock::now();
        float deltaTime = 0;
        while (running){
            frame(deltaTime);
            auto tick = Clock::now();
            deltaTime = std::chrono::duration_cast<FpSeconds>(tick - lastTick).count();
            lastTick = tick;
            if (deltaTime < timePerFrame){
                SDL_Delay((Uint32) ((timePerFrame - deltaTime) / 1000));
            }
        }
#endif
    }

    void SDLRenderer::stopEventLoop() {
        running = false;
    }

    void SDLRenderer::setWindowSize(int width, int height) {
        windowWidth = width;
        windowHeight = height;
        if (window!= nullptr){
            SDL_SetWindowSize(window, width, height);
        }
    }

    void SDLRenderer::setWindowTitle(std::string title) {
        windowTitle = title;
        if (window != nullptr) {
            SDL_SetWindowTitle(window, title.c_str());
        }
    }

    SDL_Window *SDLRenderer::getSDLWindow() {
        return window;
    }


}
