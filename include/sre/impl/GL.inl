/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include <iostream>
#include <sstream>
#include <SDL_video.h>
#include <SDL.h>

void  checkGLError(const char* title) {
    for(GLenum err; (err = glGetError()) != GL_NO_ERROR;)
    {
		if (err != GL_NONE)
		{
			if (title) std::cerr << title << std::endl;
		}
        //Process/log the error.
        switch (err){
            case GL_INVALID_ENUM:
				
                std::cerr << "GL_INVALID_ENUM"<<std::endl;
                break;
            case GL_INVALID_VALUE:
                std::cerr << "GL_INVALID_VALUE"<<std::endl;
                break;
            case GL_INVALID_OPERATION:
                std::cerr << "GL_INVALID_OPERATION"<<std::endl;
                break;
            case GL_OUT_OF_MEMORY:
                std::cerr << "GL_OUT_OF_MEMORY"<<std::endl;
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cerr << "GL_INVALID_FRAMEBUFFER_OPERATION"<<std::endl;
                break;

        }
    }
}

bool getMaximumOpenGLSupport(int * major_, int * minor_){
    Uint32 subsystem_init = SDL_WasInit(SDL_INIT_EVERYTHING);
    if (!(subsystem_init & SDL_INIT_VIDEO)) {
        SDL_Init(SDL_INIT_VIDEO);
    }
    int major[] = {1,1,1,1,1,1,2,2,3,3,3,3,4,4,4,4,4,4,4};
    int minor[] = {0,1,2,3,4,5,0,1,0,1,2,3,0,1,2,3,4,5,6};
    bool core[] = {0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1};
    bool found = false;
    for (int i=18;i>=0 && !found;i--){
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, major[i]);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor[i]);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, core[i]?SDL_GL_CONTEXT_PROFILE_CORE:SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
        SDL_Window *window = SDL_CreateWindow(
                "OpenGL Version", 0, 0, 256, 256,
                SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
        if (window) {
            // Create an OpenGL context associated with the window.
            SDL_GLContext glcontext = SDL_GL_CreateContext(window);
            if (glcontext) {
                if (major[i] >= 3){
                    glGetIntegerv(GL_MAJOR_VERSION, major_);
                    glGetIntegerv(GL_MINOR_VERSION, minor_);
                } else {
                    std::string versionString = (char*)glGetString(GL_VERSION);
                    std::vector<std::string> elems;
                    std::stringstream ss(versionString);
                    std::string majorStr;
                    std::string minorStr;
                    std::getline(ss, majorStr, '.');
                    std::getline(ss, minorStr, '.');
                    *major_ = atoi(majorStr.c_str());
                    *minor_ = atoi(minorStr.c_str());
                }
                found = true;
                // Once finished with OpenGL functions, the SDL_GLContext can be deleted.
                SDL_GL_DeleteContext(glcontext);
            }
            SDL_DestroyWindow(window);
        }
    }
    return found;
}

bool hasExtension(std::string extensionName){
    std::string exts = (char*)glGetString(GL_EXTENSIONS);
    std::stringstream ss(exts);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, ' ')) {
        if (item == extensionName){
            return true;
        }
    }

    return false;
}

std::vector<std::string> listExtension(){
    std::string exts = (char*)glGetString(GL_EXTENSIONS);
    std::stringstream ss(exts);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, ' ')) {
        elems.push_back(std::move(item));
    }
    return elems;
}

bool has_sRGB(){
    static bool res = hasExtension("GL_EXT_sRGB");
    return res;
}