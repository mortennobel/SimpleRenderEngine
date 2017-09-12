/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergnesen.com/ )
 *  License: MIT
 */

#pragma once

#include "glm/glm.hpp"

#include "sre/impl/Export.hpp"

namespace sre {
    /**
     * The camera contains two important properties:
     * - view transform matrix: Contains information about location and orientation of the camera. This matrix will
     * transform geometry from world space to eye space.
     * - projection transform matrix: Contains information about the projection the camera uses (roughly equivalent to
     * which lens it uses). Generally this can either be perspective projection (with a field of view) or a orthographic
     * projection (without any perspective).
     *
     * The camera also includes information about the viewport, which defines which part of the window is used for
     * rendering (default settings is the full window (0,0) to (1,1))
     *
     * The default camera is positioned at (0,0,0) and looking down the negative z-axis. Everything inside the volume
     * between -1 to 1 is viewed.
     *
     * The coordinate system used is right-handed with y-pointing upwards.
     */
    class DllExport Camera {
    public:


        Camera();                                               // Set camera at (0,0,0) looking down the negative
                                                                // z-axis using orthographic viewing volume between -1 to 1

        void lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up); // set position of camera in world space using
                                                                // eye position of the camera
                                                                // at position that the camera looks at (must be different from pos)
                                                                // up the up axis (used for rotating camera around z-axis). Must not be parallel with view direction (at - pos).


        void setPerspectiveProjection(float fieldOfViewY, float nearPlane, float farPlane);      // set the projectionTransform to perspective projection
                                                                                                 // fieldOfViewY field of view in degrees
                                                                                                 // nearPlane near clipping plane, defines how close an object can be to the camera before clipping
                                                                                                 // farPlane far clipping plane, defines how far an object can be to the camera before clipping

        void setOrthographicProjection(float orthographicSize, float nearPlane, float farPlane); // set the projectionTransform to orthographic parallel viewing volume.
                                                                                                 // orthographicSize half the height of the view volume (the width is computed using the viewport size)
                                                                                                 // nearPlane near clipping plane, defines how close an object can be to the camera before clipping
                                                                                                 // farPlane far clipping plane, defines how far an object can be to the camera before clipping

        void setWindowCoordinates();                             // set orthographic transform and view, where the origin is located in the lower left corner
                                                                 // z depth is between -1 and 1
                                                                 // width the width of the window, if -1 uses current window size
                                                                 // height the height of the window, if -1 uses current window size

        void setViewTransform(const glm::mat4 &viewTransform);   // Set the view transform. Used to position the virtual camera position and orientation.
                                                                 // This is commonly set using lookAt


        void setProjectionTransform(const glm::mat4 &projectionTransform);                       // Set a custom projection transform. Defines the view volume and how it is projected to the screen.
                                                                                                 // This is commonly set using setPerspectiveProjection(), setOrthographicProjection() or setWindowCoordinates()


        glm::mat4 getViewTransform();                            // Get the view transform. The matrix transformation
                                                                 // contain the orientation and position of the virtual
                                                                 // camera.

        glm::mat4 getProjectionTransform(glm::uvec2 viewportSize);// Get the projection transform - used for rendering


        void setViewport(glm::vec2 offset = glm::vec2{0,0}, glm::vec2 size = glm::vec2{1,1});    // defines which part of the window is used for
                                                                                                 // rendering (default settings is the full window).
                                                                                                 // x the x coordinate of the viewport (default 0)
                                                                                                 // y the y coordinate of the viewport (default 0)
                                                                                                 // width the width of the viewport (default window width)
                                                                                                 // height the height of the viewport (default window height)
    private:
        enum class ProjectionType {
            Perspective,
            Orthographic,
            OrthographicWindow,
            Custom
        };
        ProjectionType projectionType = ProjectionType::Orthographic;
        union {
            struct {
                float fieldOfViewY;
                float nearPlane;
                float farPlane;
            } perspective;
            struct {
                float orthographicSize;
                float nearPlane;
                float farPlane;
            } orthographic;
            float customProjectionMatrix[16];

        } projectionValue;

        glm::mat4 viewTransform;

        glm::vec2 viewportOffset = glm::vec2{0,0};
        glm::vec2 viewportSize = glm::vec2{1,1};

        friend class RenderPass;

    };

}