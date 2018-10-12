/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/VR.hpp"
#include "sre/Log.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "sre/impl/VROculus.hpp"
#include "sre/impl/VROpenVR.hpp"


namespace sre
{
	VR::VR()
		:renderVR([](std::shared_ptr<sre::Framebuffer> fb, sre::Camera cam, bool leftEye)
	{
		LOG_INFO("VR::renderVR not implemented");
	})
	{
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
	
	


	std::shared_ptr<VR> VR::create(VRType vrType) {
		

		if (vrType == VRType::OculusSDK) {
#ifdef SRE_OCULUS
			return std::shared_ptr<VR>(new VROculus());
#endif
		}
		if (vrType == VRType::OpenVR){
#ifdef SRE_OPENVR
			return std::shared_ptr<VR>(new VROpenVR());
#endif
		}
		LOG_INFO("SRE not compiled with VR support");
		auto res = std::shared_ptr<VR>(nullptr);
	}
}