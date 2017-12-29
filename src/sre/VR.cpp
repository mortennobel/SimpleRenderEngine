#include "sre/VR.hpp"
#include "sre/Log.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

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
	VR::VR(VRType vrType)
		:vrType(vrType), renderVR([](std::shared_ptr<sre::Framebuffer> fb, sre::Camera cam, bool leftEye)
	{
		LOG_INFO("VR::renderVR not implemented");
	})
	{

	}

	void VR::render()
	{
		if (vrType == VRType::OpenVR){
#ifdef SRE_OPENVR
			updateHMDMatrixPose();
			renderVR(leftFB, left, true);
			vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftTex->textureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
			renderVR(rightFB, right, false);
			vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightTex->textureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
			vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);
#endif
		}
	}

	void VR::updateHMDMatrixPose()
	{
		if (vrType == VRType::OpenVR) {
#ifdef SRE_OPENVR
			vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

			m_iValidPoseCount = 0;
			m_strPoseClasses = "";
			for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
			{
				if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
				{
					m_iValidPoseCount++;
					float *f = &(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking.m[0][0]);
					glm::mat3x4 m = glm::make_mat3x4(f);
					m_rmat4DevicePose[nDevice] = glm::transpose((glm::mat4)m);
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
				m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
				left.setViewTransform(glm::inverse(mat4eyePosLeft*m_mat4HMDPose)*baseViewTransform);
				right.setViewTransform(glm::inverse(mat4eyePosRight*m_mat4HMDPose)*baseViewTransform);
			}
#endif
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
		if (vrType == VRType::OpenVR) {
#ifdef SRE_OPENVR
			const auto mat4ProjectionLeft = getHMDMatrixProjectionEye(vr::Eye_Left);
			const auto mat4ProjectionRight = getHMDMatrixProjectionEye(vr::Eye_Right);
			left.setProjectionTransform(mat4ProjectionLeft);
			right.setProjectionTransform(mat4ProjectionRight);
#endif
		}
	}
#ifdef SRE_OPENVR
	glm::mat4 VR::getHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
	{
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
		vr::HmdMatrix34_t matEyeRight = vrSystem->GetEyeToHeadTransform(nEye);
		glm::mat4 matrixObj(
			matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
			matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
			matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
			matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
		);

		return  matrixObj;
	}
#endif

	void LabelMat4(std::string s, glm::mat4 m)
	{
		m = glm::transpose(m);
		
		ImGui::InputFloat4(s.c_str(), glm::value_ptr( m[0]));
		ImGui::InputFloat4("", glm::value_ptr(m[1]));
		ImGui::InputFloat4("", glm::value_ptr(m[2]));
		ImGui::InputFloat4("", glm::value_ptr(m[3]));
	}

	void VR::debugGUI()
	{
		if (vrType == VRType::OpenVR) {
#ifdef SRE_OPENVR
		
			LabelMat4("PoseEyeL", getHMDMatrixPoseEye(vr::Hmd_Eye::Eye_Left));
			LabelMat4("PoseEyeR", getHMDMatrixPoseEye(vr::Hmd_Eye::Eye_Right));
			LabelMat4("ProjEyeL", getHMDMatrixProjectionEye(vr::Hmd_Eye::Eye_Left));
			LabelMat4("ProjEyeR", getHMDMatrixProjectionEye(vr::Hmd_Eye::Eye_Right));
			LabelMat4("HDMPose ", m_mat4HMDPose);
		
#else
			ImGui::LabelText("", "VR not enabled");
#endif		
		}
	}

	std::shared_ptr<VR> VR::create(VRType vrType) {
		auto res = std::shared_ptr<VR>(new VR(vrType));

		if (vrType == VRType::OpenVR){
#ifdef SRE_OPENVR
			vr::HmdError peError = vr::VRInitError_None;
			res->vrSystem = vr::VR_Init(&peError, vr::VRApplication_Scene);
			if (peError != vr::VRInitError_None)
			{
				res->vrSystem = nullptr;
				LOG_ERROR(vr::VR_GetVRInitErrorAsEnglishDescription(peError));
				return {nullptr};
			}
			else
			{
				auto m_strDriver = GetTrackedDeviceString(res->vrSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String);
				auto m_strDisplay = GetTrackedDeviceString(res->vrSystem, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String);
				LOG_INFO("HMD Driver: %s", m_strDriver.c_str());
				LOG_INFO("HMD Display: %s", m_strDisplay.c_str());
			}

			if (!vr::VRCompositor())
			{
				LOG_ERROR("Compositor initialization failed. See log file for details\n");
				return {nullptr};
			}
			memset(res->m_rDevClassChar, 0, sizeof(res->m_rDevClassChar));

			res->vrSystem->GetRecommendedRenderTargetSize(&res->targetSizeW, &res->targetSizeH);
			res->leftTex = Texture::create().withRGBData(nullptr, res->targetSizeW, res->targetSizeH).withGenerateMipmaps( false).withFilterSampling(false).build();

			res->leftFB = Framebuffer::create().withTexture(res->leftTex).build();
			res->rightTex = Texture::create().withRGBData(nullptr, res->targetSizeW, res->targetSizeH).withGenerateMipmaps(false).withFilterSampling(false).build();

			res->rightFB = Framebuffer::create().withTexture(res->rightTex).build();

			res->setupCameras();
			res->mat4eyePosLeft = res->getHMDMatrixPoseEye(vr::Eye_Left);
			res->mat4eyePosRight = res->getHMDMatrixPoseEye(vr::Eye_Right);
#else
			LOG_INFO("SRE not compiled with OpenVR support");
            return {nullptr};
#endif
		}
		
		return res;
	}
}