#include <iostream>

void checkGLError() {
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