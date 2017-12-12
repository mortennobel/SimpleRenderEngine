#include "sre/VR.hpp"
#include "sre/Log.hpp"
#include <glm/gtc/type_ptr.hpp>

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
		}
	}
}