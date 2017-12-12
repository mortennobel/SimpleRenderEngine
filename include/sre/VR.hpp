/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/RenderPass.hpp"
#include "glm/glm.hpp"
#include <functional>

#ifdef SRE_OPENVR
#include <openvr.h>

namespace vr {
	class IVRSystem;
}
#endif

namespace sre {
	class VR
	{
	public:
		VR();
		void render();
		std::function<void(RenderPass& r, bool leftEye)> renderVR;
	private:
		void updateHMDMatrixPose();
		Camera left;
		Camera right;
		std::shared_ptr<Framebuffer> leftFB;
		std::shared_ptr<Texture> leftTex;
		std::shared_ptr<Framebuffer> rightFB;
		std::shared_ptr<Texture> rightTex;
		uint32_t targetSizeW;
		uint32_t targetSizeH;
#ifdef SRE_OPENVR
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
	};
}