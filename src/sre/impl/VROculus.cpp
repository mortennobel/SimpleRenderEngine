/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */
#include "sre/impl/VROculus.hpp"
#include "sre/Renderer.hpp"
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include "imgui.h"

#ifdef SRE_OCULUS

 // Include the Oculus SDK
#include "OVR_CAPI_GL.h"
#include "Extras/OVR_Math.h"



namespace sre {

	struct OculusTextureBuffer
	{
		ovrSession          Session;
		ovrTextureSwapChain ColorTextureChain;
		ovrTextureSwapChain DepthTextureChain;
		GLuint              fboId;
		OVR::Sizei               texSize;

		OculusTextureBuffer(ovrSession session, OVR::Sizei size, int sampleCount) :
			Session(session),
			ColorTextureChain(nullptr),
			DepthTextureChain(nullptr),
			fboId(0),
			texSize(0, 0)
		{
			assert(sampleCount <= 1); // The code doesn't currently handle MSAA textures.

			texSize = size;

			// This texture isn't necessarily going to be a rendertarget, but it usually is.
			assert(session); // No HMD? A little odd.
			
			ovrTextureSwapChainDesc desc = {};
			desc.Type = ovrTexture_2D;
			desc.ArraySize = 1;
			desc.Width = size.w;
			desc.Height = size.h;
			desc.MipLevels = 1;
			desc.Format = OVR_FORMAT_R8G8B8A8_UNORM_SRGB;
			desc.SampleCount = sampleCount;
			desc.StaticImage = ovrFalse;

			{
				ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &ColorTextureChain);

				int length = 0;
				ovr_GetTextureSwapChainLength(session, ColorTextureChain, &length);

				if (OVR_SUCCESS(result))
				{
					for (int i = 0; i < length; ++i)
					{
						GLuint chainTexId;
						ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain, i, &chainTexId);
						glBindTexture(GL_TEXTURE_2D, chainTexId);

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					}
				}
			}

			desc.Format = OVR_FORMAT_D32_FLOAT;

			{
				ovrResult result = ovr_CreateTextureSwapChainGL(Session, &desc, &DepthTextureChain);

				int length = 0;
				ovr_GetTextureSwapChainLength(session, DepthTextureChain, &length);

				if (OVR_SUCCESS(result))
				{
					for (int i = 0; i < length; ++i)
					{
						GLuint chainTexId;
						ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain, i, &chainTexId);
						glBindTexture(GL_TEXTURE_2D, chainTexId);

						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					}
				}
			}

			glGenFramebuffers(1, &fboId);
		}

		~OculusTextureBuffer()
		{
			if (ColorTextureChain)
			{
				ovr_DestroyTextureSwapChain(Session, ColorTextureChain);
				ColorTextureChain = nullptr;
			}
			if (DepthTextureChain)
			{
				ovr_DestroyTextureSwapChain(Session, DepthTextureChain);
				DepthTextureChain = nullptr;
			}
			if (fboId)
			{
				glDeleteFramebuffers(1, &fboId);
				fboId = 0;
			}
		}

		OVR::Sizei GetSize() const
		{
			return texSize;
		}

		void SetAndClearRenderSurface()
		{
			GLuint curColorTexId;
			GLuint curDepthTexId;
			{
				int curIndex;
				ovr_GetTextureSwapChainCurrentIndex(Session, ColorTextureChain, &curIndex);
				ovr_GetTextureSwapChainBufferGL(Session, ColorTextureChain, curIndex, &curColorTexId);
			}
			{
				int curIndex;
				ovr_GetTextureSwapChainCurrentIndex(Session, DepthTextureChain, &curIndex);
				ovr_GetTextureSwapChainBufferGL(Session, DepthTextureChain, curIndex, &curDepthTexId);
			}

			glBindFramebuffer(GL_FRAMEBUFFER, fboId);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, curColorTexId, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, curDepthTexId, 0);

			glViewport(0, 0, texSize.w, texSize.h);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_FRAMEBUFFER_SRGB);
		}

		void UnsetRenderSurface()
		{
			glBindFramebuffer(GL_FRAMEBUFFER, fboId);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
		}

		void Commit()
		{
			ovr_CommitTextureSwapChain(Session, ColorTextureChain);
			ovr_CommitTextureSwapChain(Session, DepthTextureChain);
		}
	};

	OculusTextureBuffer * eyeRenderTexture[2] = { nullptr, nullptr };


	VROculus::VROculus() {
		ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
		ovrResult result = ovr_Initialize(&initParams);
		assert(OVR_SUCCESS(result) && "Failed to initialize libOVR.");
		result = ovr_Create(&session, &luid);
		assert(OVR_SUCCESS(result) && "Failed to create libOVR session.");

		hmdDesc = ovr_GetHmdDesc(session);

		// Setup Window and Graphics
		// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
		ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };

		// Make eye render buffers
		leftFB = Framebuffer::create().withName("VR_LEFT_FB").build();
		rightFB = Framebuffer::create().withName("VR_RIGHT_FB").build();
		std::shared_ptr<Framebuffer> fbs[] = { leftFB , rightFB };
		for (int eye = 0; eye < 2; eye++) {
			int pixelsPerDisplayUnits = 1;
			ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], pixelsPerDisplayUnits);
			eyeRenderTexture[eye] = new OculusTextureBuffer(session, idealTextureSize, 1);

			glDeleteFramebuffers(1, &(fbs[eye]->frameBufferObjectId));
			fbs[eye]->dirty = false;
			fbs[eye]->frameBufferObjectId = eyeRenderTexture[eye]->fboId;
		}
	}

	VROculus::~VROculus() {
		for (int eye = 0; eye < 2; ++eye)
		{
			delete eyeRenderTexture[eye];
		}
		ovr_Destroy(session);

		ovr_Shutdown();
	}


	void VROculus::render() {
		int frameIndex = Renderer::instance->getRenderStats().frame;
		updateHMDMatrixPose();
		std::shared_ptr<Framebuffer> fbs[] = { leftFB , rightFB };

		Camera* cameras[2] = { &left,&right };

		for (int eye = 0; eye < 2; eye++) {
			eyeRenderTexture[eye]->SetAndClearRenderSurface();
			// Get view and projection matrices

			float Yaw = 0;
			static OVR::Vector3f Pos2(0.0f, 0.0f, -5.0f);
			OVR::Matrix4f rollPitchYaw = OVR::Matrix4f::RotationY(Yaw);
			OVR::Matrix4f finalRollPitchYaw = OVR::Matrix4f(EyeRenderPose[eye].Orientation);
			OVR::Vector3f finalUp = finalRollPitchYaw.Transform(OVR::Vector3f(0, 1, 0));
			OVR::Vector3f finalForward = finalRollPitchYaw.Transform(OVR::Vector3f(0, 0, -1));
			OVR::Vector3f shiftedEyePos = Pos2 + rollPitchYaw.Transform(EyeRenderPose[eye].Position);

			OVR::Matrix4f view = OVR::Matrix4f::LookAtRH(shiftedEyePos, shiftedEyePos + finalForward, finalUp);
			OVR::Matrix4f proj = ovrMatrix4f_Projection(hmdDesc.DefaultEyeFov[eye], nearPlane, farPlane, ovrProjection_None);
			glm::mat4 viewG = glm::transpose(glm::make_mat4(view.M[0]));
			glm::mat4 projG = glm::transpose(glm::make_mat4(proj.M[0]));
			glm::mat4 test = glm::perspective<float>(.3f,1.0f,.1f,100.0);
			cameras[eye]->setViewTransform(viewG * baseViewTransform);
			cameras[eye]->setProjectionTransform(projG);

			renderVR(fbs[eye], *cameras[eye], eye==0);

			// Avoids an error when calling SetAndClearRenderSurface during next iteration.
			// Without this, during the next while loop iteration SetAndClearRenderSurface
			// would bind a framebuffer with an invalid COLOR_ATTACHMENT0 because the texture ID
			// associated with COLOR_ATTACHMENT0 had been unlocked by calling wglDXUnlockObjectsNV.
			eyeRenderTexture[eye]->UnsetRenderSurface();

			// Commit changes to the textures so they get picked up frame
			eyeRenderTexture[eye]->Commit();
		}

		// Do distortion rendering, Present and flush/sync
		
		ovrLayerEyeFovDepth ld = {};
		ld.Header.Type = ovrLayerType_EyeFovDepth;
		ld.Header.Flags = ovrLayerFlag_TextureOriginAtBottomLeft;   // Because OpenGL.
		ovrTimewarpProjectionDesc posTimewarpProjectionDesc = {};
		ld.ProjectionDesc = posTimewarpProjectionDesc;

		for (int eye = 0; eye < 2; ++eye)
		{
			ld.ColorTexture[eye] = eyeRenderTexture[eye]->ColorTextureChain;
			ld.DepthTexture[eye] = eyeRenderTexture[eye]->DepthTextureChain;
			ld.Viewport[eye] = OVR::Recti(eyeRenderTexture[eye]->GetSize());
			ld.Fov[eye] = hmdDesc.DefaultEyeFov[eye];
			ld.RenderPose[eye] = EyeRenderPose[eye];
			ld.SensorSampleTime = sensorSampleTime;
		}
		
		ovrLayerHeader* layers = &ld.Header;
		ovr_SubmitFrame(session, frameIndex, nullptr, &layers, 1);
	}

	void VROculus::debugGUI() {
		ImGui::InputFloat4("PoseEyeL Orien", &EyeRenderPose[0].Orientation.w);
		ImGui::InputFloat4("PoseEyeR Orien", &EyeRenderPose[1].Orientation.w);
		ImGui::InputFloat3("PoseEyeL Pos", &EyeRenderPose[0].Position.x);
		ImGui::InputFloat3("PoseEyeR Pos", &EyeRenderPose[1].Position.x);
		
	}
	
	void VROculus::updateHMDMatrixPose() {
		int frameIndex = Renderer::instance->getRenderStats().frame;
		// Call ovr_GetRenderDesc each frame to get the ovrEyeRenderDesc, as the returned values (e.g. HmdToEyePose) may change at runtime.
		eyeRenderDesc[0] = ovr_GetRenderDesc(session, ovrEye_Left, hmdDesc.DefaultEyeFov[0]);
		eyeRenderDesc[1] = ovr_GetRenderDesc(session, ovrEye_Right, hmdDesc.DefaultEyeFov[1]);

		// Get eye poses, feeding in correct IPD offset
		HmdToEyePose[0] = eyeRenderDesc[0].HmdToEyePose;
		HmdToEyePose[1] = eyeRenderDesc[1].HmdToEyePose;

		// sensorSampleTime is fed into the layer later
		ovr_GetEyePoses(session, frameIndex, ovrTrue, HmdToEyePose, EyeRenderPose, &sensorSampleTime);

		
	}

	void VROculus::setupCameras() {
		// not used here, camera is updated in render
	}

}
#endif