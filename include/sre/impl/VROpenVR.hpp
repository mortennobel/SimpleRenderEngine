/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include "sre/VR.hpp"

#ifdef SRE_OPENVR
#include <openvr.h>

namespace vr {
	class IVRSystem;
}
#endif

namespace sre {
	class VROpenVR : public VR {
	public:
		~VROpenVR() = default;
		void render() override;
		void debugGUI() override;
	protected:
		VROpenVR();
		void updateHMDMatrixPose() override;
		void setupCameras() override;

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
		friend class VR;
	};
}