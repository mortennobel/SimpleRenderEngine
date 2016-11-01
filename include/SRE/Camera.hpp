#pragma once

#include "glm/glm.hpp"

#include "SRE/impl/Export.hpp"

namespace SRE {
    /**
     * The camera contains two important properties:
     * - view transform matrix: Contains information about location and orientation of the camera. This matrix will
     * transform geometry from world space to eye space.
     * - projection transform matrix: Contains information about the projection the camera uses (roughly equivalent to
     * which lens it uses). Generally this can either be perspective projection (with a field of view) or a orthographic
     * projection (without any perspective).
     *
     * The camera also includes information about the viewport, which defines which part of the window is used for
     * rendering (default settings is the full window)
     *
     * The default camera is positioned at (0,0,0) and looking down the negative z-axis. Everything inside the volume
     * between -1 to 1 is viewed.
     */
    class DllExport Camera {
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
        /// left
        /// right
        /// bottom
        /// top
        /// nearPlane near clipping plane, defines how close an object can be to the camera before clipping
        /// farPlane far clipping plane, defines how far an object can be to the camera before clipping
        void setOrthographicProjection(float left, float right, float  bottom, float top, float nearPlane, float farPlane);

        /// set orthographic transform and view, where the origon is located in the lower left corner
        /// z depth is between -1 and 1
        /// width the width of the window, if -1 uses current window size
        /// height the height of the window, if -1 uses current window size
        void setWindowCoordinates(int width = -1, int height = -1);


        // set the view transform directly
        void setViewTransform(const glm::mat4 &viewTransform);

        // Set the projection transform directly
        void setProjectionTransform(const glm::mat4 &projectionTransform);

        /// Get the view transform - used for rendering
        glm::mat4 getViewTransform();

        /// Get the projection transform  - used for rendering
        glm::mat4 getProjectionTransform();

        /**
         * defines which part of the window is used for
         * rendering (default settings is the full window)
         * @param x the x coordinate of the viewport (default 0)
         * @param y the y coordinate of the viewport (default 0)
         * @param width the width of the viewport (default window width)
         * @param height the height of the viewport (default window height)
         */
        void setViewport(int x, int y, int width, int height);
    private:
        glm::mat4 viewTransform;
        glm::mat4 projectionTransform;
        int viewportX, viewportY, viewportWidth, viewportHeight;

        friend class SimpleRenderEngine;
    };
}