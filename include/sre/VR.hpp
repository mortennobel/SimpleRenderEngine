/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */

#include "sre/RenderPass.hpp"
#include <functional>

#ifdef SRE_OPENVR
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
#ifdef SRE_OPENVR
		vr::IVRSystem* vrSystem;
#endif
	};
}