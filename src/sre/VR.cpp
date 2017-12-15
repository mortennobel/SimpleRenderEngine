#include "sre/VR.hpp"
#include "sre/Log.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.inl>
#include <glm/gtx/string_cast.inl>

#ifdef SRE_OPENVR


std::string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char *pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}
#endif

namespace sre
{
	VR::VR()
		:renderVR([](RenderPass&,bool)
	{
		LOG_INFO("VR::renderVR not implemented");
	})
	{
		vr::HmdError peError = vr::VRInitError_None;
		vrSystem = vr::VR_Init(&peError, vr::VRApplication_Scene);
		if (peError != vr::VRInitError_None)
		{
			vrSystem = nullptr;
			LOG_ERROR(vr::VR_GetVRInitErrorAsEnglishDescription(peError)); 
			return;
		}
		else
		{
			auto m_strDriver = GetTrackedDeviceString(vrSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
			auto m_strDisplay = GetTrackedDeviceString(vrSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
			LOG_INFO("HMD Driver: %s", m_strDriver.c_str());
			LOG_INFO("HMD Display: %s", m_strDisplay.c_str());
		}

		if (!vr::VRCompositor())
		{
			LOG_ERROR("Compositor initialization failed. See log file for details\n");
			return;
		}
		memset(m_rDevClassChar, 0, sizeof(m_rDevClassChar));

		vrSystem->GetRecommendedRenderTargetSize(&targetSizeW, &targetSizeH);
		leftTex = Texture::create().withRGBData(nullptr, targetSizeW, targetSizeH).withGenerateMipmaps( false).withFilterSampling(false).build();

		leftFB = Framebuffer::create().withTexture(leftTex).build();
		rightTex = Texture::create().withRGBData(nullptr, targetSizeW, targetSizeH).withGenerateMipmaps(false).withFilterSampling(false).build();

		rightFB = Framebuffer::create().withTexture(rightTex).build();

		setupCameras();
		mat4eyePosLeft = getHMDMatrixPoseEye(vr::Eye_Left);
		mat4eyePosRight = getHMDMatrixPoseEye(vr::Eye_Right);
	}

	void VR::render()
	{
		updateHMDMatrixPose();
		{
			auto rpLeft = RenderPass::create().withCamera(left).withFramebuffer(leftFB).withGUI(false).withName("VR_Left").build();
			renderVR(rpLeft, true);
		}
		vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftTex->textureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
		{
			auto rpRight = RenderPass::create().withCamera(right).withFramebuffer(rightFB).withGUI(false).withName("VR_Right").build();
			renderVR(rpRight, false);
		}
		vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightTex->textureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

	}

	void VR::updateHMDMatrixPose()
	{
		vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);
		
		m_iValidPoseCount = 0;
		m_strPoseClasses = "";
		for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
		{
			if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
			{
				m_iValidPoseCount++;
				float *f = &(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[0][0]);
				glm::mat3x4 m = glm::make_mat3x4(f);
				m_rmat4DevicePose[nDevice] = (glm::mat4)m;
				if (m_rDevClassChar[nDevice] == 0)
				{
					switch (vrSystem->GetTrackedDeviceClass(nDevice))
					{
					case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
					case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
					case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
					case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
					case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
					default:                                       m_rDevClassChar[nDevice] = '?'; break;
					}
				}
				m_strPoseClasses += m_rDevClassChar[nDevice];
			}
		}

		if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			m_mat4HMDPose = glm::inverse(m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd]);
			left.setViewTransform(mat4eyePosLeft * m_mat4HMDPose * baseViewTransform);
			right.setViewTransform(mat4eyePosRight * m_mat4HMDPose * baseViewTransform);
		}
	}

	void  VR::lookAt(glm::vec3 eye, glm::vec3 at, glm::vec3 up)
	{
		baseViewTransform = glm::lookAt(eye, at, up);
	}

	void  VR::setViewTransform(const glm::mat4 &viewTransform)
	{
		baseViewTransform = viewTransform;
	}

	void  VR::setNearFarPlanes(float nearPlane, float farPlane)
	{
		this->nearPlane = nearPlane;
		this->farPlane = farPlane;
		setupCameras();
	}
	
	void VR::setupCameras()
	{
#ifdef SRE_OPENVR
		const auto mat4ProjectionLeft = getHMDMatrixProjectionEye(vr::Eye_Left);
		const auto mat4ProjectionRight = getHMDMatrixProjectionEye(vr::Eye_Right);
		left.setProjectionTransform(mat4ProjectionLeft);
		right.setProjectionTransform(mat4ProjectionRight);
		
#endif
	}
#ifdef SRE_OPENVR
	glm::mat4 VR::getHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
	{
		if (!vrSystem)
			return glm::mat4(1);

		vr::HmdMatrix44_t mat = vrSystem->GetProjectionMatrix(nEye, nearPlane, farPlane);

		return glm::mat4(
			mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
			mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
			mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
			mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
		);
	}

	glm::mat4 VR::getHMDMatrixPoseEye(vr::Hmd_Eye nEye)
	{
		if (!vrSystem)
			return glm::mat4(1);

		vr::HmdMatrix34_t matEyeRight = vrSystem->GetEyeToHeadTransform(nEye);
		glm::mat4 matrixObj(
			matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
			matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
			matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
			matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

		return glm::inverse( matrixObj );
	}
#endif

	void VR::debugGUI()
	{
#ifdef SRE_OPENVR
		//auto s = glm::to_string(getHMDMatrixPoseEye(vr::Hmd_Eye::Eye_Left)[0]);
		//ImGui::LabelText("PoseEyeL", s.c_str());
#else
		ImGui::LabelText("", "VR not enabled");
#endif			
	}
}