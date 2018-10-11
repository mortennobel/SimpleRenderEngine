/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */
#include "sre/RenderPass.hpp"
#include "sre/Framebuffer.hpp"
#include "sre/Camera.hpp"
#include "glm/glm.hpp"
#include <functional>

#ifdef SRE_OCULUS
#include "OVR_CAPI_GL.h"
#endif
#ifdef SRE_OPENVR
#include <openvr.h>

namespace vr {
	class IVRSystem;
}
#endif


namespace sre {

	enum class VRType {
		OpenVR,
		OculusSDK
	};

	class VR
	{
	public:
		~VR();
		static std::shared_ptr<VR> create(VRType vrType);		// Initiate VR integration. If unsuccessful
		void render();											// Update HMD cameras (position and rotation)
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
		void debugGUI();
	private:
		VRType vrType;
		VR(VRType vrType);
		
		void updateHMDMatrixPose();
		glm::mat4 baseViewTransform = glm::mat4(1);
		float nearPlane = 0.1;
		float farPlane = 100;

		Camera left;
		Camera right;
		std::shared_ptr<Framebuffer> leftFB;
		std::shared_ptr<Texture> leftTex;
		std::shared_ptr<Framebuffer> rightFB;
		std::shared_ptr<Texture> rightTex;
		uint32_t targetSizeW;
		uint32_t targetSizeH;
		void setupCameras();
#ifdef SRE_OPENVR
		glm::mat4 mat4eyePosLeft;
		glm::mat4 mat4eyePosRight;
		glm::mat4 getHMDMatrixPoseEye(vr::Hmd_Eye nEye);
		glm::mat4 getHMDMatrixProjectionEye(vr::Hmd_Eye nEye);
		vr::IVRSystem* vrSystem;
		vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
		glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
		bool m_rbShowTrackedDevice[vr::k_unMaxTrackedDeviceCount];
		int m_iValidPoseCount;
		int m_iValidPoseCount_Last;
		glm::mat4 m_mat4HMDPose;
		std::string m_strPoseClasses;                            // what classes we saw poses for this frame
		char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];   // for each device, a character representing its class
#endif
#ifdef SRE_OCULUS
		ovrSession session;
		ovrGraphicsLuid luid;
#endif
	};
}