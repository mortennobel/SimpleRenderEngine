/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */
#pragma once
#include "sre/RenderPass.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/Camera.hpp"
#include "glm/glm.hpp"
#include <functional>

namespace sre {

	enum class VRType {
		OpenVR,
		OculusSDK
	};

	class VR
	{
	public:
		virtual ~VR() = default;
		static std::shared_ptr<VR> create(VRType vrType);		// Initiate VR integration. If unsuccessful
		virtual void render() = 0;								// Update HMD cameras (position and rotation)
															 	// and invoke renderVR to render frame
		std::function<void(std::shared_ptr<sre::Framebuffer> fb, sre::Camera cam, bool leftEye)> renderVR;
																// Callback from render function to render a single eye

		void lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up); // set position of camera in world space (view transform) using
																// eye position of the camera
																// at position that the camera looks at (must be different from pos)
																// up the up axis (used for rotating camera around z-axis). Must not be parallel with view direction (at - pos).

		void setViewTransform(const glm::mat4 &viewTransform);   // Set the view transform. Used to position the virtual camera position and orientation.
																 // This is commonly set using lookAt

		void setNearFarPlanes(float nearPlane, float farPlane);
		virtual void debugGUI() = 0;
	protected:
		VR();
		
		
		glm::mat4 baseViewTransform = glm::mat4(1);
		float nearPlane = 0.1f;
		float farPlane = 100.0f;

		Camera left;
		Camera right;
		std::shared_ptr<Framebuffer> leftFB;
		std::shared_ptr<Texture> leftTex;
		std::shared_ptr<Framebuffer> rightFB;
		std::shared_ptr<Texture> rightTex;
		uint32_t targetSizeW;
		uint32_t targetSizeH;

		virtual void updateHMDMatrixPose() = 0;
		virtual void setupCameras() = 0;
	};
}