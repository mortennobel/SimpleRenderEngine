/*
 *  SimpleRenderEngine (https://github.com/mortennobel/SimpleRenderEngine)
 *
 *  Created by Morten Nobel-Jørgensen ( http://www.nobel-joergensen.com/ )
 *  License: MIT
 */
#include "sre/impl/VROculus.hpp"


#ifdef SRE_OCULUS

 // Include the Oculus SDK
#include "OVR_CAPI_GL.h"
#include "Extras/OVR_Math.h"


#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif


namespace sre {


	struct OculusTextureBuffer
	{
		ovrSession          Session;
		ovrTextureSwapChain ColorTextureChain;
		ovrTextureSwapChain DepthTextureChain;
		GLuint              fboId;
		glm::ivec2               texSize;

		OculusTextureBuffer(ovrSession session, glm::ivec2 size, int sampleCount) :
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
			desc.Width = size.x;
			desc.Height = size.y;
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

		glm::ivec2 GetSize() const
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

			glViewport(0, 0, texSize.x, texSize.y);
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


	static ovrGraphicsLuid GetDefaultAdapterLuid()
	{
		ovrGraphicsLuid luid = ovrGraphicsLuid();

#if defined(_WIN32)
		IDXGIFactory* factory = nullptr;

		if (SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&factory))))
		{
			IDXGIAdapter* adapter = nullptr;

			if (SUCCEEDED(factory->EnumAdapters(0, &adapter)))
			{
				DXGI_ADAPTER_DESC desc;

				adapter->GetDesc(&desc);
				memcpy(&luid, &desc.AdapterLuid, sizeof(luid));
				adapter->Release();
			}

			factory->Release();
		}
#endif

		return luid;
	}


	VROculus::VROculus() {
		ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
		ovrResult result = ovr_Initialize(&initParams);
		assert(OVR_SUCCESS(result) && "Failed to initialize libOVR.");
		result = ovr_Create(&session, &luid);
		assert(OVR_SUCCESS(result) && "Failed to create libOVR session.");

		ovrHmdDesc hmdDesc = ovr_GetHmdDesc(session);

		// Setup Window and Graphics
		// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
		ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };

		// Make eye render buffers
		for (int eye = 0; eye < 2; ++eye)
		{
			ovrSizei idealTextureSize = ovr_GetFovTextureSize(session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
			leftTex = Texture::create().withRGBData(nullptr, idealTextureSize.w, idealTextureSize.h).withGenerateMipmaps(false).withFilterSampling(false).build();

			leftFB = Framebuffer::create().withColorTexture(leftTex).withName("VR_LEFT_FB").build();
			rightTex = Texture::create().withRGBData(nullptr, idealTextureSize.w, idealTextureSize.h).withGenerateMipmaps(false).withFilterSampling(false).build();

			rightFB = Framebuffer::create().withColorTexture(rightTex).withName("VR_RIGHT_FB").build();
		}
	}

	VROculus::~VROculus() {
		ovr_Shutdown();
	}

	void VROculus::render() {
	}

	void VROculus::debugGUI() {
	}
	
	void VROculus::updateHMDMatrixPose() {
	}

	void VROculus::setupCameras() {
	}

}
#endif