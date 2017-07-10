//
// Created by Morten Nobel-Jørgensen on 30/06/2017.
//

#pragma once

#define SDL_MAIN_HANDLED

#include "SDL.h"


#include <functional>
#include <string>
#include "sre/Renderer.hpp"



namespace sre {

// forward declaration
class Renderer;

/// Simplifies SDL applications by abstracting away boilerplate code.
///
/// SDLRenderer is a pure helper-class, and no other class in the SimpleRenderEngine depends on it.
///
/// The class will create a window with a graphics context in the `init()` member function.
/// The `startEventLoop()` will start the event loop, which polls the event queue in the
/// beginning of each frame (and providing callbacks to `keyEvent` and `mouseEvent`), followed by a `frameUpdate(float)`
/// and a `frameRender(Renderer)`.
class DllExport SDLRenderer {
public:
    SDLRenderer();
    virtual ~SDLRenderer();

    // event handlers (assigned empty default handlers)
    std::function<void(float deltaTimeSec)> frameUpdate;
    // Subscript be render events. The `Renderer::swapFrame()` is automatically invoked after the callback.
    std::function<void(Renderer* renderer)> frameRender;

    // Call back of `SDL_KEYDOWN` and `SDL_KEYUP`.
    std::function<void(SDL_Event& e)> keyEvent;

    // Call back of `SDL_MOUSEMOTION`, `SDL_MOUSEBUTTONDOWN`, `SDL_MOUSEBUTTONUP`, `SDL_MOUSEWHEEL`.
    std::function<void(SDL_Event& e)> mouseEvent;

    // Call back of `SDL_CONTROLLERAXISMOTION`, `SDL_CONTROLLERBUTTONDOWN`, `SDL_CONTROLLERBUTTONUP`,
    // `SDL_CONTROLLERDEVICEADDED`, `SDL_CONTROLLERDEVICEREMOVED` and `SDL_CONTROLLERDEVICEREMAPPED`.
    std::function<void(SDL_Event& e)> controllerEvent;

    // Call back of `SDL_JOYAXISMOTION`, `SDL_JOYBALLMOTION`, `SDL_JOYHATMOTION`, `SDL_JOYBUTTONDOWN`,
    // `SDL_JOYBUTTONUP`, `SDL_JOYDEVICEADDED`, `SDL_JOYDEVICEREMOVED`.
    std::function<void(SDL_Event& e)> joystickEvent;

    // Call back of `SDL_FINGERDOWN`, `SDL_FINGERUP`, `SDL_FINGERMOTION`.
    std::function<void(SDL_Event& e)> touchEvent;

    // Invoked if unhandled event
    std::function<void(SDL_Event& e)> otherEvent;

    // Create the window and the graphics context (instantiates the sre::Renderer). Note that most
    // other sre classes requires the graphics content to be created before they can be used (e.g. a Shader cannot be
    // created before `init()`).
    void init(uint32_t sdlInitFlag = SDL_INIT_EVERYTHING);

    void setWindowTitle(std::string title);
    void setWindowSize(glm::ivec2 size);

    glm::ivec2 getWindowSize();

    // Start the event loop. Note that this member function in usually blocking (until the `stopEventLoop()` has been
    // called). Using Emscripten the event loop is not blocking (but internally using a callback function), which means
    // that when using Emscripten avoid allocating objects on the stack (see examples for a workaround).
    void startEventLoop();

    // The render loop will stop running when the frame is complete.
    void stopEventLoop();

    SDL_Window *getSDLWindow();
private:

    void frame(float deltaTimeSec);

    Renderer* r;
    SDLRenderer(const SDLRenderer&) = delete;
    std::string windowTitle;

    float timePerFrame = 1.0f/60;

    bool running = false;
    int windowWidth = 800;
    int windowHeight = 600;
    SDL_Window *window = nullptr;

    friend class SDLRendererInternal;
};

}
