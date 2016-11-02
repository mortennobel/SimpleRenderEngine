//
// Created by morten on 01/08/16.
//

#include "SRE/Camera.hpp"

#include "SRE/impl/GL.hpp"

#include "SRE/SimpleRenderEngine.hpp"
#include <glm/gtc/matrix_transform.hpp>


namespace SRE{
    Camera::Camera()
    : viewTransform{1.0f},projectionTransform {1.0f}, viewportX{0}, viewportY{0}
    {
        if (SimpleRenderEngine::instance){
            SDL_GetWindowSize(SimpleRenderEngine::instance->window,&viewportWidth,&viewportHeight);
        }
    }

    void Camera::setPerspectiveProjection(float fieldOfViewY, float viewportWidth,float viewportHeight, float nearPlane, float farPlane) {
        projectionTransform = glm::perspectiveFov<float>	(	glm::radians( fieldOfViewY),
                                                          viewportWidth,
                                                          viewportHeight,
                                                          nearPlane,
                                                          farPlane);
    }

    void Camera::setOrthographicProjection(float left, float right, float bottom, float top, float nearPlane, float farPlane) {
        projectionTransform = glm::ortho<float>	(left, right, bottom, top, nearPlane, farPlane);
    }

    void Camera::setWindowCoordinates(int width, int height){
        int w,h;
        SDL_GetWindowSize(SimpleRenderEngine::instance->window,&w,&h);
        if (width == -1){
            width = w;
        }
        if (height == -1){
            height = h;
        }
        setOrthographicProjection(0.0f,(float)width,0.0f,(float)height,1.0f,-1.0f);
        viewTransform = glm::mat4(1);
    }

    void Camera::lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up) {
        viewTransform = glm::lookAt<float>(eye, at, up);
    }

    glm::mat4 Camera::getViewTransform() {
        return viewTransform;
    }

    glm::mat4 Camera::getProjectionTransform() {
        return projectionTransform;
    }

    void Camera::setViewTransform(const glm::mat4 &viewTransform) {
        Camera::viewTransform = viewTransform;
    }

    void Camera::setProjectionTransform(const glm::mat4 &projectionTransform) {
        Camera::projectionTransform = projectionTransform;
    }

    void Camera::setViewport(int x, int y, int width, int height) {
        viewportX = x;
        viewportY = y;
        viewportWidth = width;
        viewportHeight = height;
        if (SimpleRenderEngine::instance->getCamera() == this){
            glViewport(x, y, width, height);
            glScissor(x, y, width, height);
        }
    }
}
