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
#endif


namespace sre {
	class VROculus : public VR {
	public:
		~VROculus();
		void render() override;
		void debugGUI() override;
	protected:
		VROculus();
		void updateHMDMatrixPose() override;
		void setupCameras() override;

		ovrSession session;
		ovrGraphicsLuid luid;
		friend class VR;
	};
}