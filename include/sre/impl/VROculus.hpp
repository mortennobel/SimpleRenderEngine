/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#pragma once

#include "sre/VR.hpp"

#ifdef SRE_OCULUS
#include "OVR_CAPI_GL.h"


namespace sre {
	class VROculus : public VR {
	public:
		~VROculus();
		void render() override;
		void debugGUI() override;
	protected:
		VROculus();
		void updateHMDMatrixPose();
		void setupCameras() override;

		ovrSession session;
		ovrGraphicsLuid luid;
		ovrHmdDesc hmdDesc;
		double sensorSampleTime;  
		ovrEyeRenderDesc eyeRenderDesc[2];
		ovrPosef EyeRenderPose[2];
		ovrPosef HmdToEyePose[2];
		friend class VR;
	};
}
#endif