//
// Created by morten on 01/08/16.
//

#include "sre/Camera.hpp"

#include "sre/impl/GL.hpp"

#include "sre/Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <sre/Camera.hpp>
#include <glm/gtc/type_ptr.hpp>


namespace sre {
    Camera::Camera()
    : viewTransform{1.0f}
    {
        projectionValue.orthographic.orthographicSize = 1;
        projectionValue.orthographic.nearPlane = -1;
        projectionValue.orthographic.farPlane = 1;
        if (! Renderer::instance ){
            throw std::runtime_error("Cannot instantiate sre::Camera before sre::Renderer is created.");
        }
    }

    void Camera::setPerspectiveProjection(float fieldOfViewY, float nearPlane, float farPlane) {
        projectionValue.perspective.fieldOfViewY = glm::radians( fieldOfViewY);
        projectionValue.perspective.nearPlane    = nearPlane;
        projectionValue.perspective.farPlane     = farPlane;
        projectionType = ProjectionType::Perspective;
    }

    void Camera::setOrthographicProjection(float orthographicSize, float nearPlane, float farPlane) {
        projectionValue.orthographic.orthographicSize  = orthographicSize;
        projectionValue.orthographic.nearPlane = nearPlane;
        projectionValue.orthographic.farPlane  = farPlane;
        projectionType = ProjectionType::Orthographic;
    }

    void Camera::setWindowCoordinates(){
        projectionType = ProjectionType::OrthographicWindow;
    }

    void Camera::lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up) {
        setViewTransform(glm::lookAt<float>(eye, at, up));
    }

    glm::mat4 Camera::getViewTransform() {
        return viewTransform;
    }

    glm::mat4 Camera::getProjectionTransform(glm::uvec2 viewportSize) {
        switch (projectionType){
            case ProjectionType::Custom:
                return glm::make_mat4(projectionValue.customProjectionMatrix);
            case ProjectionType::Orthographic:
            {
                float aspect = viewportSize.x/(float)viewportSize.y;
                float sizeX = aspect * projectionValue.orthographic.orthographicSize;
                return glm::ortho<float>	(-sizeX, sizeX, -projectionValue.orthographic.orthographicSize, projectionValue.orthographic.orthographicSize, projectionValue.orthographic.nearPlane, projectionValue.orthographic.farPlane);
            }
            case ProjectionType::OrthographicWindow:
                return glm::ortho<float>	(0, viewportSize.x, 0, viewportSize.y, 1.0f,-1.0f);
            case ProjectionType::Perspective:
                return glm::perspectiveFov<float>(projectionValue.perspective.fieldOfViewY,
                                                  viewportSize.x,
                                                  viewportSize.y,
                                                  projectionValue.perspective.nearPlane,
                                                  projectionValue.perspective.farPlane);
        }
        return glm::mat4(1);
    }

    void Camera::setViewTransform(const glm::mat4 &viewTransform) {
        Camera::viewTransform = viewTransform;
    }

    void Camera::setProjectionTransform(const glm::mat4 &projectionTransform) {
        memcpy(projectionValue.customProjectionMatrix, glm::value_ptr(projectionTransform), sizeof(glm::mat4));
        projectionType = ProjectionType::Custom;
    }

    void Camera::setViewport(glm::vec2 offset, glm::vec2 size) {
        viewportOffset = offset;
        viewportSize = size;
    }
}
