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

#ifdef SRE_OCULUS
 // Include the Oculus SDK
#include "OVR_CAPI_GL.h"
#include "Extras/OVR_Math.h"


#if defined(_WIN32)
#include <dxgi.h> // for GetDefaultAdapterLuid
#pragma comment(lib, "dxgi.lib")
#endif


using namespace OVR;

struct OGL
{
	static const bool       UseDebugContext = false;

	HWND                    Window;
	HDC                     hDC;
	HGLRC                   WglContext;
	// OVR::GLEContext         GLEContext;
	bool                    Running;
	bool                    Key[256];
	int                     WinSizeW;
	int                     WinSizeH;
	GLuint                  fboId;
	HINSTANCE               hInstance;

	static LRESULT CALLBACK WindowProc(_In_ HWND hWnd, _In_ UINT Msg, _In_ WPARAM wParam, _In_ LPARAM lParam)
	{
		OGL *p = reinterpret_cast<OGL *>(GetWindowLongPtr(hWnd, 0));
		switch (Msg)
		{
		case WM_KEYDOWN:
			p->Key[wParam] = true;
			break;
		case WM_KEYUP:
			p->Key[wParam] = false;
			break;
		case WM_DESTROY:
			p->Running = false;
			break;
		default:
			return DefWindowProcW(hWnd, Msg, wParam, lParam);
		}
		if ((p->Key['Q'] && p->Key[VK_CONTROL]) || p->Key[VK_ESCAPE])
		{
			p->Running = false;
		}
		return 0;
	}

	OGL() :
		Window(nullptr),
		hDC(nullptr),
		WglContext(nullptr),
		//GLEContext(),
		Running(false),
		WinSizeW(0),
		WinSizeH(0),
		fboId(0),
		hInstance(nullptr)
	{
		// Clear input
		for (int i = 0; i < sizeof(Key) / sizeof(Key[0]); ++i)
			Key[i] = false;
	}

	~OGL()
	{
		ReleaseDevice();
		CloseWindow();
	}

	bool InitWindow(HINSTANCE hInst, LPCWSTR title)
	{
		hInstance = hInst;
		Running = true;

		WNDCLASSW wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_CLASSDC;
		wc.lpfnWndProc = WindowProc;
		wc.cbWndExtra = sizeof(struct OGL *);
		wc.hInstance = GetModuleHandleW(NULL);
		wc.lpszClassName = L"ORT";
		RegisterClassW(&wc);

		// adjust the window size and show at InitDevice time
		Window = CreateWindowW(wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, 0, 0, 0, 0, 0, 0, hInstance, 0);
		if (!Window) return false;

		SetWindowLongPtr(Window, 0, LONG_PTR(this));

		hDC = GetDC(Window);

		return true;
	}

	void CloseWindow()
	{
		if (Window)
		{
			if (hDC)
			{
				ReleaseDC(Window, hDC);
				hDC = nullptr;
			}
			DestroyWindow(Window);
			Window = nullptr;
			UnregisterClassW(L"OGL", hInstance);
		}
	}

	// Note: currently there is no way to get GL to use the passed pLuid
	bool InitDevice(int vpW, int vpH, const LUID* /*pLuid*/, bool windowed = true)
	{
		UNREFERENCED_PARAMETER(windowed);

		WinSizeW = vpW;
		WinSizeH = vpH;

		//RECT size = { 0, 0, vpW, vpH };
		//AdjustWindowRect(&size, WS_OVERLAPPEDWINDOW, false);
		//const UINT flags = SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW;
		//if (!SetWindowPos(Window, nullptr, 0, 0, size.right - size.left, size.bottom - size.top, flags))
		//	return false;
		/*
		PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARBFunc = nullptr;
		PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARBFunc = nullptr;
		{
			// First create a context for the purpose of getting access to wglChoosePixelFormatARB / wglCreateContextAttribsARB.
			PIXELFORMATDESCRIPTOR pfd;
			memset(&pfd, 0, sizeof(pfd));
			pfd.nSize = sizeof(pfd);
			pfd.nVersion = 1;
			pfd.iPixelType = PFD_TYPE_RGBA;
			pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
			pfd.cColorBits = 32;
			pfd.cDepthBits = 16;
			int pf = ChoosePixelFormat(hDC, &pfd);
			// VALIDATE(pf, "Failed to choose pixel format.");

			// VALIDATE(SetPixelFormat(hDC, pf, &pfd), "Failed to set pixel format.");

			HGLRC context = wglCreateContext(hDC);
			// VALIDATE(context, "wglCreateContextfailed.");
			// VALIDATE(wglMakeCurrent(hDC, context), "wglMakeCurrent failed.");

			wglChoosePixelFormatARBFunc = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
			wglCreateContextAttribsARBFunc = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
			assert(wglChoosePixelFormatARBFunc && wglCreateContextAttribsARBFunc);

			wglDeleteContext(context);
		}

		// Now create the real context that we will be using.
		int iAttributes[] =
		{
			// WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			GL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 16,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB, GL_TRUE,
			0, 0
		};

		float fAttributes[] = { 0, 0 };
		int   pf = 0;
		UINT  numFormats = 0;

		// VALIDATE(wglChoosePixelFormatARBFunc(hDC, iAttributes, fAttributes, 1, &pf, &numFormats),
		//	"wglChoosePixelFormatARBFunc failed.");

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		// VALIDATE(SetPixelFormat(hDC, pf, &pfd), "SetPixelFormat failed.");

		GLint attribs[16];
		int   attribCount = 0;
		if (UseDebugContext)
		{
			attribs[attribCount++] = WGL_CONTEXT_FLAGS_ARB;
			attribs[attribCount++] = WGL_CONTEXT_DEBUG_BIT_ARB;
		}

		attribs[attribCount] = 0;

		WglContext = wglCreateContextAttribsARBFunc(hDC, 0, attribs);
		// VALIDATE(wglMakeCurrent(hDC, WglContext), "wglMakeCurrent failed.");

		OVR::GLEContext::SetCurrentContext(&GLEContext);
		GLEContext.Init();

		glGenFramebuffers(1, &fboId);

		glEnable(GL_DEPTH_TEST);
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);

		if (UseDebugContext && GLE_ARB_debug_output)
		{
			glDebugMessageCallbackARB(DebugGLCallback, NULL);
			if (glGetError())
			{
				assert( false && ("glDebugMessageCallbackARB failed."));
			}

			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

			// Explicitly disable notification severity output.
			glDebugMessageControlARB(GL_DEBUG_SOURCE_API, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
		}
		*/
		return true;
	}

	bool HandleMessages(void)
	{
		MSG msg;
		while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return Running;
	}

	void Run(bool(*MainLoop)(bool retryCreate))
	{
		while (HandleMessages())
		{
			// true => we'll attempt to retry for ovrError_DisplayLost
			if (!MainLoop(true))
				break;
			// Sleep a bit before retrying to reduce CPU load while the HMD is disconnected
			Sleep(10);
		}
	}

	void ReleaseDevice()
	{
		if (fboId)
		{
			glDeleteFramebuffers(1, &fboId);
			fboId = 0;
		}
		if (WglContext)
		{
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(WglContext);
			WglContext = nullptr;
		}
		//GLEContext.Shutdown();
	}

	static void GLAPIENTRY DebugGLCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
	{
		UNREFERENCED_PARAMETER(source);
		UNREFERENCED_PARAMETER(type);
		UNREFERENCED_PARAMETER(id);
		UNREFERENCED_PARAMETER(severity);
		UNREFERENCED_PARAMETER(length);
		UNREFERENCED_PARAMETER(message);
		UNREFERENCED_PARAMETER(userParam);
		printf("Message from OpenGL: %s\n", message);
	}
};

// Global OpenGL state
static OGL Platform;


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

#endif

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

	VR::~VR() {
		if (vrType == VRType::OculusSDK) {
#ifdef SRE_OCULUS
			ovr_Shutdown();
#endif
		}
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
				left.setViewTransform(glm::inverse(m_mat4HMDPose*mat4eyePosLeft)*baseViewTransform);
				right.setViewTransform(glm::inverse(m_mat4HMDPose*mat4eyePosRight)*baseViewTransform);
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

		if (vrType == VRType::OculusSDK) {
#ifdef SRE_OCULUS
			ovrInitParams initParams = { ovrInit_RequestVersion | ovrInit_FocusAware, OVR_MINOR_VERSION, NULL, 0, 0 };
			ovrResult result = ovr_Initialize(&initParams);
			assert(OVR_SUCCESS(result) && "Failed to initialize libOVR.");
			result = ovr_Create(&res->session, &res->luid);
			assert(OVR_SUCCESS(result) && "Failed to create libOVR session.");

			ovrHmdDesc hmdDesc = ovr_GetHmdDesc(res->session);

			// Setup Window and Graphics
			// Note: the mirror window can be any size, for this sample we use 1/2 the HMD resolution
			ovrSizei windowSize = { hmdDesc.Resolution.w / 2, hmdDesc.Resolution.h / 2 };
			bool initialized = Platform.InitDevice(windowSize.w, windowSize.h, reinterpret_cast<LUID*>(&res->luid));
			assert(initialized);

			// Make eye render buffers
			for (int eye = 0; eye < 2; ++eye)
			{
				ovrSizei idealTextureSize = ovr_GetFovTextureSize(res->session, ovrEyeType(eye), hmdDesc.DefaultEyeFov[eye], 1);
				eyeRenderTexture[eye] = new OculusTextureBuffer(res->session, idealTextureSize, 1);

				if (!eyeRenderTexture[eye]->ColorTextureChain || !eyeRenderTexture[eye]->DepthTextureChain)
				{
					// if (retryCreate) goto Done;
					// VALIDATE(false, "Failed to create texture.");
					assert(false && "Failed to create texture.");
				}
			}

#endif
		}
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

			res->leftFB = Framebuffer::create().withColorTexture(res->leftTex).withName("VR_LEFT_FB").build();
			res->rightTex = Texture::create().withRGBData(nullptr, res->targetSizeW, res->targetSizeH).withGenerateMipmaps(false).withFilterSampling(false).build();

			res->rightFB = Framebuffer::create().withColorTexture(res->rightTex).withName("VR_RIGHT_FB").build();

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