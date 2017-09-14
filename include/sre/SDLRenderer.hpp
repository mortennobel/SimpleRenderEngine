/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#ifdef _SDL_H
#error SDL should not be included before SDLRenderer
#endif

#define SDL_MAIN_HANDLED

#include "SDL.h"


#include <functional>
#include <string>
#include "sre/Renderer.hpp"



namespace sre {

// forward declaration
class Renderer;

// Simplifies SDL applications by abstracting away boilerplate code.
//
// SDLRenderer is a pure helper-class, and no other class in the SimpleRenderEngine depends on it.
//
// The class will create a window with a graphics context in the `init()` member function.
// The `startEventLoop()` will start the event loop, which polls the event queue in the
// beginning of each frame (and providing callbacks to `keyEvent` and `mouseEvent`), followed by a `frameUpdate(float)`
// and a `frameRender()`.
class DllExport SDLRenderer {
public:
    SDLRenderer();
    virtual ~SDLRenderer();

    // event handlers (assigned empty default handlers)
    std::function<void(float deltaTimeSec)> frameUpdate;        // Callback every frame with time since last callback in seconds
    std::function<void()> frameRender;                          // Callback be render events - called after frameUpdate. The `Renderer::swapFrame()` is automatically invoked after the callback.
    std::function<void(SDL_Event& e)> keyEvent;                 // Callback of `SDL_KEYDOWN` and `SDL_KEYUP`.
    std::function<void(SDL_Event& e)> mouseEvent;               // Callback of `SDL_MOUSEMOTION`, `SDL_MOUSEBUTTONDOWN`, `SDL_MOUSEBUTTONUP`, `SDL_MOUSEWHEEL`.
    std::function<void(SDL_Event& e)> controllerEvent;          // Callback of `SDL_CONTROLLERAXISMOTION`, `SDL_CONTROLLERBUTTONDOWN`, `SDL_CONTROLLERBUTTONUP`,
                                                                // `SDL_CONTROLLERDEVICEADDED`, `SDL_CONTROLLERDEVICEREMOVED` and `SDL_CONTROLLERDEVICEREMAPPED`.
    std::function<void(SDL_Event& e)> joystickEvent;            // Callback of `SDL_JOYAXISMOTION`, `SDL_JOYBALLMOTION`, `SDL_JOYHATMOTION`, `SDL_JOYBUTTONDOWN`,
                                                                // `SDL_JOYBUTTONUP`, `SDL_JOYDEVICEADDED`, `SDL_JOYDEVICEREMOVED`.
    std::function<void(SDL_Event& e)> touchEvent;               // Callback of `SDL_FINGERDOWN`, `SDL_FINGERUP`, `SDL_FINGERMOTION`.
    std::function<void(SDL_Event& e)> otherEvent;               // Invoked if unhandled SDL event

    void init(uint32_t sdlInitFlag = SDL_INIT_EVERYTHING,       // Create the window and the graphics context (instantiates the sre::Renderer). Note that most
              uint32_t sdlWindowFlags=SDL_WINDOW_ALLOW_HIGHDPI  // other sre classes requires the graphics content to be created before they can be used (e.g. a Shader cannot be
                                        | SDL_WINDOW_OPENGL     // created before `init()`).
                                        | SDL_WINDOW_RESIZABLE);




    void setWindowTitle(std::string title);
    void setWindowSize(glm::ivec2 size);

    void setFullscreen(bool enabled = true);                    // Toggle fullscreen mode (default mode is windowed). Not supported in Emscripten
    bool isFullscreen();                                        //

    void setMouseCursorVisible(bool enabled = true);            // Show/hide mouse cursor. Not supported in Emscripten
    bool isMouseCursorVisible();                                // GUI should not be rendered when mouse cursor is not visible (this would force the mouse cursor to appear again)

    bool setMouseCursorLocked(bool enabled = true);             // Lock the mouse cursor, such that mouse cursor motion is detected, (while position remains fixed). Not supported in Emscripten
    bool isMouseCursorLocked();                                 // Locking the mouse cursor automatically hides the mouse cursor

    void startEventLoop();                                      // Start the event loop. Note that this member function in usually blocking (until the `stopEventLoop()` has been
                                                                // called). Using Emscripten the event loop is not blocking (but internally using a callback function), which means
                                                                // that when using Emscripten avoid allocating objects on the stack (see examples for a workaround).

    void stopEventLoop();                                       // The render loop will stop running when the frame is complete.

    SDL_Window *getSDLWindow();                                 // Get a pointer to SDL_Window
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
