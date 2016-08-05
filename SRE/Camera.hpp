#pragma once

#include "glm/glm.hpp"

namespace SRE {
    class Camera {
    public:
        /// Set camera at (0,0,0) looking down the negative z-axis using orthographic viewing volume between -1 to 1
        Camera();
        /// set position of camera in world space using
        /// eye position of the camera
        /// at position that the camera looks at (must be different from pos)
        /// up the up axis (used for rotating camera around z-axis). Must not be parallel with view direction (at - pos).
        void lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up);

        /// set the projectionTransform to perspective projection
        /// fieldOfViewY field of view in degrees
        /// viewportWidth width of the current viewport
        /// viewportHeight height of the current viewport
        /// nearPlane near clipping plane, defines how close an object can be to the camera before clipping
        /// farPlane far clipping plane, defines how far an object can be to the camera before clipping
        void setPerspectiveProjection(float fieldOfViewY, float viewportWidth,float viewportHeight, float nearPlane, float farPlane);

        /// set the projectionTransform to orthographic parallel viewing volume.
        /// fieldOfViewY field of view in degrees
        /// viewportWidth width of the current viewport
        /// viewportHeight height of the current viewport
        /// nearPlane near clipping plane, defines how close an object can be to the camera before clipping
        /// farPlane far clipping plane, defines how far an object can be to the camera before clipping
        void setOrthographicProjection(float left, float right, float  bottom, float top, float zNear, float zFar);

        // set the view transform directly
        void setViewTransform(const glm::mat4 &viewTransform);

        // Set the projection transform directly
        void setProjectionTransform(const glm::mat4 &projectionTransform);

        /// Get the view transform - used for rendering
        glm::mat4 getViewTransform();

        /// Get the projection transform  - used for rendering
        glm::mat4 getProjectionTransform();

        void setViewport(int x, int y, int width, int height);
    private:
        glm::mat4 viewTransform;
        glm::mat4 projectionTransform;
        int viewportX, viewportY, viewportWidth, viewportHeight;

        friend class SimpleRenderEngine;
    };
}