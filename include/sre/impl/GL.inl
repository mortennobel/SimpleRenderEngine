/*
 *  SimpleRenderEngine
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#include <iostream>
#include <sstream>

void  checkGLError() {
    for(GLenum err; (err = glGetError()) != GL_NO_ERROR;)
    {
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