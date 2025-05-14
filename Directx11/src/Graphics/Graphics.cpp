#include "pch.h"

#include "Graphics.h"
#include "Vertex.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx12.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "SampleRenderer.h"
#include <dxgidebug.h>
#include "nvrhi/utils.h"
#include "nvrhi/d3d12.h"
#include "nvrhi/validation.h"
#include "nvrhi/common/resource.h"
#include "directx/d3d12.h"

namespace Engine
{
#define HR_RETURN(hr) if(FAILED(hr)) ErrorLogger::Log(hr, "FAILED"); return false;
DefaultMessageCallback& DefaultMessageCallback::GetInstance()
{
	static DefaultMessageCallback Instance;
	return Instance;
}
void DefaultMessageCallback::message(nvrhi::MessageSeverity severity, const char* messageText)
{
	std::string severityString;
	switch (severity)
	{
		case nvrhi::MessageSeverity::Info: severityString = "INFO: "; break;
		case nvrhi::MessageSeverity::Error: severityString = "ERROR: "; break;
		case nvrhi::MessageSeverity::Warning: severityString = "WARNING: "; break;
		case nvrhi::MessageSeverity::Fatal: severityString = "FATAL: "; break;
	}


	Engine::ErrorLogger::Log(std::string(severityString) + std::string(messageText));
}

Graphics::~Graphics()
{
	DestroyDeviceAndSwapChain();
	/*for (int i = 0; i < 6; i++)
	{
		delete HDRIFramebufferRTV[i];
	}
	delete[] HDRIFramebufferRTV;*/
}
void Graphics::DestroyDeviceAndSwapChain()
{
	m_RhiSwapChainBuffers.clear();
	m_RendererString.clear();

	ReleaseRenderTargets();

	m_NvrhiDevice = nullptr;

	for (auto fenceEvent : m_FrameFenceEvents)
	{
		WaitForSingleObject(fenceEvent, INFINITE);
		CloseHandle(fenceEvent);
	}

	m_FrameFenceEvents.clear();

	if (m_SwapChain)
	{
		m_SwapChain->SetFullscreenState(false, nullptr);
	}

	m_SwapChainBuffers.clear();

	m_FrameFence = nullptr;
	m_SwapChain = nullptr;
	m_GraphicsQueue = nullptr;
	m_ComputeQueue = nullptr;
	m_CopyQueue = nullptr;
	m_Device12 = nullptr;
}
void Graphics::ReleaseRenderTargets()
{
	if (m_NvrhiDevice)
	{
		// Make sure that all frames have finished rendering
		m_NvrhiDevice->waitForIdle();

		// Release all in-flight references to the render targets
		m_NvrhiDevice->runGarbageCollection();
	}

	// Set the events so that WaitForSingleObject in OneFrame will not hang later
	for (auto e : m_FrameFenceEvents)
		SetEvent(e);

	// Release the old buffers because ResizeBuffers requires that
	m_RhiSwapChainBuffers.clear();
	m_SwapChainBuffers.clear();
}


void Graphics::ReportLiveObjects()
{
	nvrhi::RefCountPtr<IDXGIDebug> pDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug));

	if (pDebug)
	{
		DXGI_DEBUG_RLO_FLAGS flags = (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_DETAIL);
		HRESULT hr = pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, flags);
		if (FAILED(hr))
		{
			Engine::ErrorLogger::Log(hr, "ReportLiveObjects failed, HRESULT" );
		}
	}
}

bool Graphics::CreateInstanceInternal()
{

	if (!m_DxgiFactory2)
	{
		#ifndef NDEBUG
		
		HRESULT hres = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_DxgiFactory2));
		#else
		HRESULT hres = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_DxgiFactory2));

		#endif
		if (hres != S_OK)
		{
			Engine::ErrorLogger::Log(hres, "ERROR in CreateDXGIFactory2.\n"
				"For more info, get log from debug D3D runtime: (1) Install DX SDK, and enable Debug D3D from DX Control Panel Utility. (2) Install and start DbgView. (3) Try running the program again.\n");
			return false;
		}
	}

	return true;
}
bool Graphics::EnumerateAdapters(std::vector<AdapterInfo>& outAdapters)
{
	if (!m_DxgiFactory2)
		return false;

	outAdapters.clear();

	while (true)
	{
		nvrhi::RefCountPtr<IDXGIAdapter> adapter;
		HRESULT hr = m_DxgiFactory2->EnumAdapters(uint32_t(outAdapters.size()), &adapter);
		if (FAILED(hr))
			return true;

		DXGI_ADAPTER_DESC desc;
		hr = adapter->GetDesc(&desc);
		if (FAILED(hr))
			return false;

		AdapterInfo adapterInfo;

		adapterInfo.name = GetAdapterName(desc);
		adapterInfo.dxgiAdapter = adapter;
		adapterInfo.vendorID = desc.VendorId;
		adapterInfo.deviceID = desc.DeviceId;
		adapterInfo.dedicatedVideoMemory = desc.DedicatedVideoMemory;

		AdapterInfo::LUID luid;
		static_assert(luid.size() == sizeof(desc.AdapterLuid));
		memcpy(luid.data(), &desc.AdapterLuid, luid.size());
		adapterInfo.luid = luid;

		outAdapters.push_back(std::move(adapterInfo));
	}
}

bool Graphics::CreateDevice()
{
	#ifndef NDEBUG
		nvrhi::RefCountPtr<ID3D12Debug> pDebug;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug));

		if (SUCCEEDED(hr))
			pDebug->EnableDebugLayer();
		else
			Engine::ErrorLogger::Log(hr, "Cannot enable DX12 debug runtime, ID3D12Debug is not available.");
	#endif

	if (enableGPUValidation)
	{
		nvrhi::RefCountPtr<ID3D12Debug3> debugController3;
		HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugController3));

		if (SUCCEEDED(hr))
			debugController3->SetEnableGPUBasedValidation(true);
		else
			Engine::ErrorLogger::Log(hr, "Cannot enable GPU-based validation, ID3D12Debug3 is not available.");
	}

	int adapterIndex = -1;

	if (adapterIndex < 0)
		adapterIndex = 0;

	if (FAILED(m_DxgiFactory2->EnumAdapters(adapterIndex, &m_DxgiAdapter)))
	{
		if (adapterIndex == 0)
			Engine::ErrorLogger::Log("Cannot find any DXGI adapters in the system.");
		else
			Engine::ErrorLogger::Log("The specified DXGI adapter does not exist.");

		return false;
	}

	{
		DXGI_ADAPTER_DESC aDesc;
		m_DxgiAdapter->GetDesc(&aDesc);

		m_RendererString = GetAdapterName(aDesc);
	}


	HRESULT hr = D3D12CreateDevice(
		m_DxgiAdapter,
		D3D_FEATURE_LEVEL_11_1,
		IID_PPV_ARGS(&m_Device12));

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "D3D12CreateDevice failed");
		return false;
	}


	if (enableDebugRuntime)
	{
		nvrhi::RefCountPtr<ID3D12InfoQueue> pInfoQueue;
		m_Device12->QueryInterface(&pInfoQueue);

		if (pInfoQueue)
		{
#ifdef _DEBUG
			if (enableDebugRuntime)
				pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			pInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
#endif

			D3D12_MESSAGE_ID disableMessageIDs[] = {
				D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
				D3D12_MESSAGE_ID_COMMAND_LIST_STATIC_DESCRIPTOR_RESOURCE_DIMENSION_MISMATCH, // descriptor validation doesn't understand acceleration structures

			};

			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.pIDList = disableMessageIDs;
			filter.DenyList.NumIDs = sizeof(disableMessageIDs) / sizeof(disableMessageIDs[0]);
			pInfoQueue->AddStorageFilterEntries(&filter);
		}
	}

	D3D12_COMMAND_QUEUE_DESC queueDesc;
	ZeroMemory(&queueDesc, sizeof(queueDesc));
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.NodeMask = 1;
	hr = m_Device12->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_GraphicsQueue));
	HR_RETURN(hr)
		m_GraphicsQueue->SetName(L"Graphics Queue");

	if (false)
	{
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COMPUTE;
		hr = m_Device12->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_ComputeQueue));
		HR_RETURN(hr)
			m_ComputeQueue->SetName(L"Compute Queue");
	}

	if (false)
	{
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_COPY;
		hr = m_Device12->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CopyQueue));
		HR_RETURN(hr)
			m_CopyQueue->SetName(L"Copy Queue");
	}

	nvrhi::d3d12::DeviceDesc deviceDesc;
	deviceDesc.errorCB = &DefaultMessageCallback::GetInstance();
	deviceDesc.pDevice = m_Device12;
	deviceDesc.pGraphicsCommandQueue = m_GraphicsQueue;
	deviceDesc.pComputeCommandQueue = m_ComputeQueue;
	deviceDesc.pCopyCommandQueue = m_CopyQueue;
#if AFTERMATH
	deviceDesc.aftermathEnabled = m_DeviceParams.enableAftermath;
#endif
	deviceDesc.logBufferLifetime = true;
	deviceDesc.enableHeapDirectlyIndexed = true;

	m_NvrhiDevice = nvrhi::d3d12::createDevice(deviceDesc);
	
	if (enableDebugRuntime)
	{
		m_NvrhiDevice = nvrhi::validation::createValidationLayer(m_NvrhiDevice);
	}

	return true;
}

bool Graphics::CreateSwapChain(HWND hwnd)
{
	UINT windowStyle = m_DeviceParams.startFullscreen
		? (WS_POPUP | WS_SYSMENU | WS_VISIBLE)
		: m_DeviceParams.startMaximized
		? (WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE)
		: (WS_OVERLAPPEDWINDOW | WS_VISIBLE);

	RECT rect = { 0, 0, LONG(m_DeviceParams.backBufferWidth), LONG(m_DeviceParams.backBufferHeight) };
	AdjustWindowRect(&rect, windowStyle, FALSE);

	m_hWnd = hwnd;

	HRESULT hr = E_FAIL;

	RECT clientRect;
	GetClientRect(m_hWnd, &clientRect);
	UINT width = clientRect.right - clientRect.left;
	UINT height = clientRect.bottom - clientRect.top;

	ZeroMemory(&m_SwapChainDesc, sizeof(m_SwapChainDesc));
	m_SwapChainDesc.Width = width;
	m_SwapChainDesc.Height = height;
	m_SwapChainDesc.SampleDesc.Count = m_DeviceParams.swapChainSampleCount;
	m_SwapChainDesc.SampleDesc.Quality = 0;
	m_SwapChainDesc.BufferUsage = m_DeviceParams.swapChainUsage;
	m_SwapChainDesc.BufferCount = m_DeviceParams.swapChainBufferCount;
	m_SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	m_SwapChainDesc.Flags = m_DeviceParams.allowModeSwitch ? DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH : 0;

	// Special processing for sRGB swap chain formats.
	// DXGI will not create a swap chain with an sRGB format, but its contents will be interpreted as sRGB.
	// So we need to use a non-sRGB format here, but store the true sRGB format for later framebuffer creation.
	switch (m_DeviceParams.swapChainFormat)  // NOLINT(clang-diagnostic-switch-enum)
	{
	case nvrhi::Format::SRGBA8_UNORM:
		m_SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case nvrhi::Format::SBGRA8_UNORM:
		m_SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	default:
		m_SwapChainDesc.Format = nvrhi::d3d12::convertFormat(m_DeviceParams.swapChainFormat);
		break;
	}

	nvrhi::RefCountPtr<IDXGIFactory5> pDxgiFactory5;
	if (SUCCEEDED(m_DxgiFactory2->QueryInterface(IID_PPV_ARGS(&pDxgiFactory5))))
	{
		BOOL supported = 0;
		if (SUCCEEDED(pDxgiFactory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &supported, sizeof(supported))))
			m_TearingSupported = (supported != 0);
	}

	if (m_TearingSupported)
	{
		m_SwapChainDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;
	}

	m_FullScreenDesc = {};
	m_FullScreenDesc.RefreshRate.Numerator = m_DeviceParams.refreshRate;
	m_FullScreenDesc.RefreshRate.Denominator = 1;
	m_FullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
	m_FullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	m_FullScreenDesc.Windowed = !m_DeviceParams.startFullscreen;

	nvrhi::RefCountPtr<IDXGISwapChain1> pSwapChain1;
	hr = m_DxgiFactory2->CreateSwapChainForHwnd(m_GraphicsQueue, m_hWnd, &m_SwapChainDesc, &m_FullScreenDesc, nullptr, &pSwapChain1);
	HR_RETURN(hr)

		hr = pSwapChain1->QueryInterface(IID_PPV_ARGS(&m_SwapChain));
	HR_RETURN(hr)

		if (!CreateRenderTargets())
			return false;

	hr = m_Device12->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_FrameFence));
	HR_RETURN(hr)

		for (UINT bufferIndex = 0; bufferIndex < m_SwapChainDesc.BufferCount; bufferIndex++)
		{
			m_FrameFenceEvents.push_back(CreateEvent(nullptr, false, true, nullptr));
		}

	return true;
}

bool Graphics::CreateRenderTargets()
{
	m_SwapChainBuffers.resize(m_SwapChainDesc.BufferCount);
	m_RhiSwapChainBuffers.resize(m_SwapChainDesc.BufferCount);

	for (UINT n = 0; n < m_SwapChainDesc.BufferCount; n++)
	{
		const HRESULT hr = m_SwapChain->GetBuffer(n, IID_PPV_ARGS(&m_SwapChainBuffers[n]));
		HR_RETURN(hr)

			nvrhi::TextureDesc textureDesc;
		textureDesc.width = m_DeviceParams.backBufferWidth;
		textureDesc.height = m_DeviceParams.backBufferHeight;
		textureDesc.sampleCount = m_DeviceParams.swapChainSampleCount;
		textureDesc.sampleQuality = m_DeviceParams.swapChainSampleQuality;
		textureDesc.format = m_DeviceParams.swapChainFormat;
		textureDesc.debugName = "SwapChainBuffer";
		textureDesc.isRenderTarget = true;
		textureDesc.isUAV = false;
		textureDesc.initialState = nvrhi::ResourceStates::Present;
		textureDesc.keepInitialState = true;

		m_RhiSwapChainBuffers[n] = m_NvrhiDevice->createHandleForNativeTexture(nvrhi::ObjectTypes::D3D12_Resource, nvrhi::Object(m_SwapChainBuffers[n]), textureDesc);
	}

	return true;
}

void Graphics::ResizeSwapChain()
{
	ReleaseRenderTargets();

	if (!m_NvrhiDevice)
		return;

	if (!m_SwapChain)
		return;

	const HRESULT hr = m_SwapChain->ResizeBuffers(m_DeviceParams.swapChainBufferCount,
		m_DeviceParams.backBufferWidth,
		m_DeviceParams.backBufferHeight,
		m_SwapChainDesc.Format,
		m_SwapChainDesc.Flags);

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "ResizeBuffers failed");
	}

	bool ret = CreateRenderTargets();
	if (!ret)
	{
		ErrorLogger::Log("CreateRenderTarget failed");
	}
}

bool Graphics::BeginFrame()
{
	DXGI_SWAP_CHAIN_DESC1 newSwapChainDesc;
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC newFullScreenDesc;
	if (SUCCEEDED(m_SwapChain->GetDesc1(&newSwapChainDesc)) && SUCCEEDED(m_SwapChain->GetFullscreenDesc(&newFullScreenDesc)))
	{
		if (m_FullScreenDesc.Windowed != newFullScreenDesc.Windowed)
		{
			BackBufferResizing();

			m_FullScreenDesc = newFullScreenDesc;
			m_SwapChainDesc = newSwapChainDesc;
			m_DeviceParams.backBufferWidth = newSwapChainDesc.Width;
			m_DeviceParams.backBufferHeight = newSwapChainDesc.Height;

			ResizeSwapChain();
			BackBufferResized();
		}

	}

	auto bufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	WaitForSingleObject(m_FrameFenceEvents[bufferIndex], INFINITE);

	return true;
}

void Graphics::BackBufferResized()
{


	uint32_t backBufferCount = GetBackBufferCount();
	m_SwapChainBuffers.resize(backBufferCount);
	for (uint32_t index = 0; index < backBufferCount; index++)
	{
		m_SwapChainBuffers[index] = m_NvrhiDevice->createFramebuffer(
			nvrhi::FramebufferDesc().addColorAttachment(GetBackBuffer(index)));
	}
}
nvrhi::ITexture* Graphics::GetBackBuffer(uint32_t index)
{
	if (index < m_RhiSwapChainBuffers.size())
		return m_RhiSwapChainBuffers[index];
	return nullptr;
}
void Graphics::BackBufferResizing()
{
	m_SwapChainBuffers.clear();
}
bool Graphics::Present()
{
	/*if (!m_windowVisible)
		return true;*/

	auto bufferIndex = m_SwapChain->GetCurrentBackBufferIndex();

	UINT presentFlags = 0;
	if (!m_DeviceParams.vsyncEnabled && m_FullScreenDesc.Windowed && m_TearingSupported)
		presentFlags |= DXGI_PRESENT_ALLOW_TEARING;

	HRESULT result = m_SwapChain->Present(m_DeviceParams.vsyncEnabled ? 1 : 0, presentFlags);

	m_FrameFence->SetEventOnCompletion(m_FrameCount, m_FrameFenceEvents[bufferIndex]);
	m_GraphicsQueue->Signal(m_FrameFence, m_FrameCount);
	m_FrameCount++;
	return SUCCEEDED(result);
}


uint32_t Graphics::GetCurrentBackBufferIndex()
{
	return m_SwapChain->GetCurrentBackBufferIndex();
}

uint32_t Graphics::GetBackBufferCount()
{
	return m_SwapChainDesc.BufferCount;
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	
	windowWidth = width;
	windowHeight = height;
	if (!InitializeDirectX(hwnd))
		return false;
	physicsController.initialise();

	if (!InitializeShaders())
		return false;
	if (!InitializeScene())
		return false;

	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init((ID3D12Device*)(m_NvrhiDevice->getNativeObject(nvrhi::ObjectTypes::D3D12_Device)).pointer, 
		2, 
		DXGI_FORMAT_B8G8R8A8_UNORM, 
		ImguiHeap.Get(),
		ImguiHeap->GetCPUDescriptorHandleForHeapStart(),
		ImguiHeap->GetGPUDescriptorHandleForHeapStart());
	ImGui::StyleColorsDark();


	return true;
}
void Graphics::PhysicsUpdate()
{
	physicsController.Update();
}



void Graphics::RenderFrame()
{

}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 1000.0f);
	CreateDevice();
	m_CommandList = m_NvrhiDevice->createCommandList();
	nvrhi::BindingLayoutDesc Binddesc;
	Binddesc.bindings = {
		nvrhi::BindingLayoutItem::Texture_SRV(0),
		nvrhi::BindingLayoutItem::Texture_SRV(1)
	};
	ImguiBindingLayout = m_NvrhiDevice->createBindingLayout(Binddesc);

	D3D12_DESCRIPTOR_HEAP_DESC heapdesc = {};
	heapdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapdesc.NumDescriptors = 1;                        // at least 1 for the font texture
	heapdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	heapdesc.NodeMask = 0;


	m_Device12->CreateDescriptorHeap(&heapdesc, IID_PPV_ARGS(&ImguiHeap));
	// D3D11_SAMPLER_DESC HDRIsampDesc = {};
	// HDRIsampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	// HDRIsampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	// HDRIsampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	// HDRIsampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;


	//D3D11_SAMPLER_DESC presamplerDesc = {};
	//presamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;         // Smooth trilinear filtering
	//presamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;           // Clamp to edge to avoid seams
	//presamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	//presamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	//presamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	//presamplerDesc.MinLOD = 0;
	//presamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	//presamplerDesc.MipLODBias = 0.0f;
	//presamplerDesc.MaxAnisotropy = 1;


	return true;
}

bool Graphics::InitializeShaders()
{
	return true;
}

bool Graphics::InitializeScene()
{

	//GBuffer--------------------------------------------------------------
	// Common settings for G-buffer textures

	
	
	// Texture for Position (32-bit floating point RGBA format to store position accurately)
	
	nvrhi::TextureDesc textureDesc = {};
	textureDesc.width = windowWidth;
	textureDesc.height = windowHeight;
	textureDesc.mipLevels = 1;
	textureDesc.arraySize = 1;
	textureDesc.sampleCount = 1;
	textureDesc.isRenderTarget = true;
	textureDesc.isShaderResource = true;
	textureDesc.format = nvrhi::Format::RGBA32_FLOAT;
	positionTexture = m_NvrhiDevice->createTexture(textureDesc);
	
	
	// Texture for Normal (32-bit RGBA for storing normal data accurately)
	textureDesc.format = nvrhi::Format::RGBA16_FLOAT;
	NormalTexture = m_NvrhiDevice->createTexture(textureDesc);

	// Texture for Normal (32-bit RGBA for storing normal data accurately)
	textureDesc.format = nvrhi::Format::RGBA8_UNORM;
	SpecularTexture = m_NvrhiDevice->createTexture(textureDesc);

	// Texture for Albedo (8-bit RGBA as it's only color data)
	textureDesc.format = nvrhi::Format::RGBA8_UNORM;
	DiffuseTexture = m_NvrhiDevice->createTexture(textureDesc);

	// Now create Render Target Views (RTVs) for each texture
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	nvrhi::TextureDesc DepthtextureDesc = {};
	DepthtextureDesc.width = windowWidth;
	DepthtextureDesc.height = windowHeight;
	DepthtextureDesc.mipLevels = 1;
	DepthtextureDesc.arraySize = 1;
	DepthtextureDesc.sampleCount = 1;

	const nvrhi::Format depthFormats[] = {
		nvrhi::Format::D24S8,
		nvrhi::Format::D32S8,
		nvrhi::Format::D32,
		nvrhi::Format::D16 };

	const nvrhi::FormatSupport depthFeatures =
		nvrhi::FormatSupport::Texture |
		nvrhi::FormatSupport::DepthStencil |
		nvrhi::FormatSupport::ShaderLoad;

	DepthtextureDesc.format = nvrhi::utils::ChooseFormat(m_NvrhiDevice, depthFeatures, depthFormats, std::size(depthFormats));
	DepthTexture = m_NvrhiDevice->createTexture(textureDesc);

	nvrhi::FramebufferDesc fbdesc = {};
	fbdesc.addColorAttachment(positionTexture);
	fbdesc.addColorAttachment(NormalTexture);
	fbdesc.addColorAttachment(SpecularTexture);
	fbdesc.addColorAttachment(DiffuseTexture);
	fbdesc.setDepthAttachment(DepthTexture);
	GBUfferFrameBuffer = m_NvrhiDevice->createFramebuffer(fbdesc);

	// Position SRV

	
	

	//D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	//depthStencilDesc.DepthEnable = FALSE;              
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS; 

	//depthStencilDesc.StencilEnable = FALSE; // Optional — disables stencil too

	// HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilStateDisabled);
	//HDRI-----------------------------------------
	// D3D11_SAMPLER_DESC samplerDesc = {};
	// samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	// samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  
	// samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  
	// samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  
	// samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	// samplerDesc.MinLOD = 0;
	// samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//hr = device->CreateSamplerState(&samplerDesc, &HDRIsamplerState);
	
	
	//D3D11_DEPTH_STENCIL_DESC HDRIdepthStencilDesc = {};
	//HDRIdepthStencilDesc.DepthEnable = TRUE;
	//HDRIdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//HDRIdepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	//
	//hr = device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled);
	

	nvrhi::TextureDesc texdesc = {};
	texdesc.width = 512;
	texdesc.height = 512;
	texdesc.mipLevels = 0;// 0 = generate full mip chain
	texdesc.arraySize = 6;// 6 faces
	texdesc.format = nvrhi::Format::RGBA16_FLOAT;// HDR-capable
	texdesc.isShaderResource = true;
	texdesc.isRenderTarget = true;
	texdesc.dimension = nvrhi::TextureDimension::TextureCube;
	HDRIFramebufferTexture = m_NvrhiDevice->createTexture(texdesc);

	for (uint32_t face = 0; face < 6; ++face)
	{
		nvrhi::FramebufferDesc fbDesc;
		nvrhi::FramebufferAttachment fbattach = {};
		fbattach.format = texdesc.format;
		fbattach.texture = HDRIFramebufferTexture;
		fbattach.setArraySlice(face);
		fbDesc.addColorAttachment(fbattach);
		faceFbs[face] = m_NvrhiDevice->createFramebuffer(fbDesc);
	}
	
	nvrhi::TextureDesc irradiancetexDesc = {};
	irradiancetexDesc.width = 32; 
	irradiancetexDesc.height = 32;
	irradiancetexDesc.mipLevels = 1;
	irradiancetexDesc.arraySize = 6; // 6 faces
	irradiancetexDesc.format = nvrhi::Format::RGBA16_FLOAT; // HDR-capable
	irradiancetexDesc.isShaderResource = true;
	irradiancetexDesc.isRenderTarget = true;
	irradiancetexDesc.dimension = nvrhi::TextureDimension::TextureCube;
	IrradiancemapTexture = m_NvrhiDevice->createTexture(irradiancetexDesc);

	
	for (uint32_t face = 0; face < 6; ++face)
	{
		nvrhi::FramebufferDesc fbDesc;
		nvrhi::FramebufferAttachment fbattach = {};
		fbattach.format = irradiancetexDesc.format;
		fbattach.setArraySlice(face);
		fbDesc.addColorAttachment(fbattach);
		irradianceFBs[face] = m_NvrhiDevice->createFramebuffer(fbDesc);
	}

	

	nvrhi::TextureDesc irradiancedepthDesc = {};
	irradiancedepthDesc.width = 32;
	irradiancedepthDesc.height = 32;
	irradiancedepthDesc.mipLevels = 1;
	irradiancedepthDesc.arraySize = 1;
	irradiancedepthDesc.format = nvrhi::Format::D24S8;
	irradiancedepthDesc.isShaderResource = false;
	irradiancedepthDesc.isRenderTarget = false;

	irradiancedepthStencilBuffer = m_NvrhiDevice->createTexture(irradiancedepthDesc);

	
	/*D3D11_DEPTH_STENCIL_DESC IrradiancedepthStencilDesc = {};
	IrradiancedepthStencilDesc.DepthEnable = TRUE;
	IrradiancedepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	IrradiancedepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;*/


	//hr = device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled);

	//D3D11_DEPTH_STENCIL_DESC skyboxdepthStencilDesc = {};
	//skyboxdepthStencilDesc.DepthEnable = TRUE;
	//skyboxdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 
	//skyboxdepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	//device->CreateDepthStencilState(&skyboxdepthStencilDesc, &depthStencilSkyboxState);
	
	
	const UINT baseSize = 128;
	const UINT mipLevels = static_cast<UINT>(std::floor(std::log2(baseSize))) + 1;

	nvrhi::TextureDesc pretexDesc = {};
	pretexDesc.width = baseSize;
	pretexDesc.height = baseSize;
	pretexDesc.mipLevels = mipLevels;
	pretexDesc.arraySize = 6; // Cube has 6 faces
	pretexDesc.format = nvrhi::Format::RGBA16_FLOAT;
	
	pretexDesc.isRenderTarget = true;
	pretexDesc.isShaderResource = true;
	pretexDesc.dimension = nvrhi::TextureDimension::TextureCube;
	PrefilteringTexture = m_NvrhiDevice->createTexture(pretexDesc);

	PrefilteringFBs.resize(mipLevels);
	for (int i = 0 ; i < PrefilteringFBs.size(); i++)
	{
		PrefilteringFBs[i].resize(6);
	}

	for (UINT mip = 0; mip < mipLevels; ++mip)
	{
		for (UINT face = 0; face < 6; ++face)
		{

			nvrhi::FramebufferDesc fbDesc;
			nvrhi::FramebufferAttachment fbattach = {};
			fbattach.format = pretexDesc.format;
			fbattach.texture = PrefilteringTexture;
			fbattach.subresources.baseArraySlice = face;
			fbattach.subresources.baseMipLevel = mip;
			fbattach.setArraySlice(face);
			fbDesc.addColorAttachment(fbattach);
			PrefilteringFBs[mip][face] = m_NvrhiDevice->createFramebuffer(fbDesc);
		}
	}

	nvrhi::TextureDesc BRDFDesc = {};
	BRDFDesc.width = 512; 
	BRDFDesc.height = 512;
	BRDFDesc.mipLevels = 1;
	BRDFDesc.arraySize = 1; 
	BRDFDesc.format = nvrhi::Format::RG32_FLOAT;
	BRDFDesc.isShaderResource = true;
	BRDFDesc.isRenderTarget = true;

	BRDFTexture = m_NvrhiDevice->createTexture(BRDFDesc);

	////CASCADED SHADOW MAPS----------------------------------------
	//for (int i = 0; i < NUM_CASCADES; ++i)
	//{
	//	D3D11_TEXTURE2D_DESC desc = {};
	//	desc.Width = depthMapResolution;
	//	desc.Height = depthMapResolution;
	//	desc.MipLevels = 1;
	//	desc.ArraySize = 1;
	//	desc.Format = DXGI_FORMAT_R32_TYPELESS;
	//	desc.SampleDesc.Count = 1;
	//	desc.Usage = D3D11_USAGE_DEFAULT;
	//	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	//	shadowTex.resize(NUM_CASCADES);
	//	hr = device->CreateTexture2D(&desc, nullptr, shadowTex[i].GetAddressOf());
	//	shadowDSVs.resize(NUM_CASCADES);
	//	// Depth view
	//	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	//	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	//	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	//	hr = device->CreateDepthStencilView(shadowTex[i].Get(), &dsvDesc, &shadowDSVs[i]);
	//	shadowSRVs.resize(NUM_CASCADES);
	//	// SRV for shader access
	//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	//	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//	srvDesc.Texture2D.MipLevels = 1;
	//	hr = device->CreateShaderResourceView(shadowTex[i].Get(), &srvDesc, &shadowSRVs[i]);
	//}
	
	//D3D11_DEPTH_STENCIL_DESC ShadowdepthStencilDesc = {};
	//ShadowdepthStencilDesc.DepthEnable = TRUE;                                 // Enable depth testing
	//ShadowdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;        // Allow depth writes
	//ShadowdepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;            // Standard comparison

	//ShadowdepthStencilDesc.StencilEnable = FALSE;                              // We don’t use stencil here

	// hr = device->CreateDepthStencilState(&ShadowdepthStencilDesc, shadowDepthStencilState.GetAddressOf());


	// D3D11_BLEND_DESC blendDesc = {};
	// blendDesc.AlphaToCoverageEnable = FALSE;
	// blendDesc.IndependentBlendEnable = FALSE;

	// D3D11_RENDER_TARGET_BLEND_DESC rtBlend = {};
	// rtBlend.BlendEnable = TRUE;
	// rtBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	// rtBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	// rtBlend.BlendOp = D3D11_BLEND_OP_ADD;
	// rtBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
	// rtBlend.DestBlendAlpha = D3D11_BLEND_ZERO;
	// rtBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	// rtBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	// blendDesc.RenderTarget[0] = rtBlend;

	/* D3D11_RASTERIZER_DESC rasterizerDesc;
	 ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	 rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	 rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	 
	 hr = this->device->CreateRasterizerState(&rasterizerDesc, this->DebugLineState.GetAddressOf());*/
	 //D3D11_TEXTURE2D_DESC ShadowtexDesc = {};
	 //ShadowtexDesc.Width = depthMapResolution;
	 //ShadowtexDesc.Height = depthMapResolution;
	 //ShadowtexDesc.MipLevels = 1;
	 //ShadowtexDesc.ArraySize = shadowSRVs.size();
	 //ShadowtexDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Or your format
	 //ShadowtexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	 //ShadowtexDesc.MiscFlags = 0;
	 //ShadowtexDesc.Usage = D3D11_USAGE_DEFAULT;
	 //ShadowtexDesc.SampleDesc.Count = 1;

	 //
	 //device->CreateTexture2D(&ShadowtexDesc, nullptr, &ShadowtextureArray);


	 //for (UINT i = 0; i < shadowSRVs.size(); ++i)
	 //{
		// UINT dstSubresource = D3D11CalcSubresource(0, i, 1);
		// deviceContext->CopySubresourceRegion(
		//	 ShadowtextureArray.Get(), dstSubresource, 0, 0, 0,
		//	 shadowTex[i].Get(), 0, nullptr);
	 //}

	 //D3D11_SHADER_RESOURCE_VIEW_DESC ShadowsrvDesc = {};
	 //ShadowsrvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Or matching your data
	 //ShadowsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	 //ShadowsrvDesc.Texture2DArray.MostDetailedMip = 0;
	 //ShadowsrvDesc.Texture2DArray.MipLevels = 1;
	 //ShadowsrvDesc.Texture2DArray.FirstArraySlice = 0;
	 //ShadowsrvDesc.Texture2DArray.ArraySize = shadowSRVs.size();

	 //
	 //device->CreateShaderResourceView(ShadowtextureArray.Get(), &ShadowsrvDesc, &ShadowtextureArraySRV);

	/*nvrhi::SamplerDesc sampDesc = {};
	sampDesc.minFilter = true;
	sampDesc.magFilter = true;
	sampDesc.addressU = nvrhi::SamplerAddressMode::Border;
	sampDesc.addressV = nvrhi::SamplerAddressMode::Border;
	sampDesc.addressW = nvrhi::SamplerAddressMode::Border;
	sampDesc.reductionType = nvrhi::SamplerReductionType::Comparison;
	sampDesc.borderColor = {1.0,1.0,1.0,1.0};
	shadowSampler = m_NvrhiDevice->createSampler(sampDesc)*/
	
	
	
	 nvrhi::TextureDesc desc = {};
	 desc.width = depthMapResolution;
	 desc.height = depthMapResolution;
	 desc.mipLevels = 1;
	 desc.arraySize = 1;
	 desc.format = nvrhi::Format::R32_FLOAT;
	 desc.isShaderResource = true;
	 desc.isRenderTarget = true;
	 DirectionalshadowTex = m_NvrhiDevice->createTexture(desc);

	 // Depth view
	/* D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	 dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	 dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	 hr = device->CreateDepthStencilView(DirectionalshadowTex.Get(), &dsvDesc, &DirectionalshadowDSVs);*/
	 // SRV for shader access

	return true;




}
std::vector<XMFLOAT3> Graphics::GetFrustumCornersWorldSpace(const XMMATRIX& viewProj)
{
	XMMATRIX inv = XMMatrixInverse(nullptr, viewProj);

	std::vector<XMFLOAT3> frustumCorners;
	frustumCorners.reserve(8);

	for (int x = 0; x < 2; ++x)
	{
		for (int y = 0; y < 2; ++y)
		{
			for (int z = 0; z < 2; ++z)
			{
				XMVECTOR corner = XMVectorSet(
					x == 0 ? -1.0f : 1.0f,
					y == 0 ? -1.0f : 1.0f,
					z == 0 ? -1.0f : 1.0f,  // DirectX clip space z: [0, 1]
					1.0f
				);

				XMVECTOR world = XMVector4Transform(corner, inv);
				world = XMVectorDivide(world, XMVectorSplatW(world));

				XMFLOAT3 pt;
				XMStoreFloat3(&pt, world);
				frustumCorners.push_back(pt);
			}
		}
	}
	return frustumCorners;
}


std::vector<XMFLOAT3> Graphics::getFrustumCornersWorldSpace(const XMMATRIX& proj, const XMMATRIX& view)
{
	return GetFrustumCornersWorldSpace( view * proj);
}

XMMATRIX Graphics::getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
	// 1. Camera frustum corners in world space
	const XMMATRIX cameraProj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(90.0f),
		static_cast<float>(windowWidth) / static_cast<float>(windowHeight),
		nearPlane, farPlane);

	const XMMATRIX viewProj = camera.GetViewMatrix() * cameraProj;
	std::vector<XMFLOAT3> corners = GetFrustumCornersWorldSpace(viewProj);

	// 2. Calculate frustum center
	XMVECTOR center = XMVectorZero();
	for (const auto& pt : corners)
		center += XMLoadFloat3(&pt);
	center /= static_cast<float>(corners.size());

	// 3. Setup light view matrix
	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&LightDir));
	XMVECTOR lightPos = center - lightDir * 100.0f;
	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMMATRIX lightView = XMMatrixLookAtLH(lightPos, center, up);

	// 4. Transform frustum corners into light space
	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minY = FLT_MAX, maxY = -FLT_MAX;
	float minZ = FLT_MAX, maxZ = -FLT_MAX;

	for (const auto& corner : corners)
	{
		XMVECTOR cornerVec = XMLoadFloat3(&corner);
		XMVECTOR trf = XMVector3TransformCoord(cornerVec, lightView);
		XMFLOAT3 pt;
		XMStoreFloat3(&pt, trf);

		minX = std::min(minX, pt.x); maxX = std::max(maxX, pt.x);
		minY = std::min(minY, pt.y); maxY = std::max(maxY, pt.y);
		minZ = std::min(minZ, pt.z); maxZ = std::max(maxZ, pt.z);
	}

	// 5. Snap orthographic bounds to shadow map texel size
	const float worldUnitsPerTexel = (maxX - minX) / static_cast<float>(depthMapResolution);

	minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
	maxX = std::floor(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
	minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
	maxY = std::floor(maxY / worldUnitsPerTexel) * worldUnitsPerTexel;

	// 6. Optional clamp if area is too small (prevents light-space collapse)
	const float minSize = 100.0f;
	if ((maxX - minX) < minSize)
	{
		float cx = 0.5f * (minX + maxX);
		minX = cx - minSize * 0.5f;
		maxX = cx + minSize * 0.5f;
	}
	if ((maxY - minY) < minSize)
	{
		float cy = 0.5f * (minY + maxY);
		minY = cy - minSize * 0.5f;
		maxY = cy + minSize * 0.5f;
	}

	// 7. Expand Z range
	const float zMult = 10.0f;
	if (minZ < 0) minZ *= zMult; else minZ /= zMult;
	if (maxZ < 0) maxZ /= zMult; else maxZ *= zMult;

	// 8. Final orthographic projection
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);

	return XMMatrixTranspose(lightView * lightProj); // Transposed for HLSL
}

std::vector<XMMATRIX> Graphics::getLightSpaceMatrices()
{
	//std::vector<XMMATRIX> ret;
	//for (size_t i = 0; i < NUM_CASCADES; i++)
	//{
	//	if (i == 0)
	//	{
	//		ret.push_back(getLightSpaceMatrix(0.1, shadowCascadeLevels[i]));
	//	}
	//	else if (i < NUM_CASCADES-1)
	//	{
	//		ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
	//	}
	//	else
	//	{
	//		ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], 1000));
	//	}
	//}
	//return ret;
}
std::vector<XMVECTOR> Graphics::getFrustumCornersWorldSpace(const XMMATRIX& projview)
{
	XMMATRIX inv = XMMatrixInverse(nullptr, projview);
	std::vector<XMVECTOR> frustumCorners;
	frustumCorners.reserve(8);

	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				// Clip space point
				XMVECTOR pt = XMVectorSet(
					2.0f * x - 1.0f,
					2.0f * y - 1.0f,
					2.0f * z - 1.0f,
					1.0f
				);

				// Transform to world space
				XMVECTOR world = XMVector4Transform(pt, inv);
				world = XMVectorDivide(world, XMVectorSplatW(world)); // Divide by w

				frustumCorners.push_back(world);
			}
		}
	}

	return frustumCorners;
}
//void Graphics::ClearDepthStencil(nvrhi::TextureHandle stencil)
//{
//	this->m_CommandList->clearDepthStencilTexture(stencil,, 1.0f, 0);
//
//}
//void Graphics::SetInputLayout(ID3D11InputLayout* layout)
//{
//	this->deviceContext->IASetInputLayout(layout);
//
//}
//void Graphics::SetTopology(D3D11_PRIMITIVE_TOPOLOGY top)
//{
//	this->deviceContext->IASetPrimitiveTopology(top);
//}
//void Graphics::SetRasterizerState()
//{
//	this->deviceContext->RSSetState(this->rasterizerstate.Get());
//}
//void Graphics::SetDepthStencilState()
//{
//	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
//}
//void Graphics::SetBlendState()
//{
//	this->deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
//}
//void Graphics::SetSamplers()
//{
//	this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
//}
//void Graphics::SetPSShader(ID3D11PixelShader* shader)
//{
//	this->deviceContext->PSSetShader(shader, NULL, 0);
//}
//void Graphics::SetVSShader(ID3D11VertexShader* shader)
//{
//	this->deviceContext->VSSetShader(shader, NULL, 0);
//}
//
//void Graphics::SetPSConstantBuffers(UINT startSlot, UINT NumOfBuffers, ID3D11Buffer*const* ppBuffer)
//{
//	this->deviceContext->PSSetConstantBuffers(startSlot, NumOfBuffers, ppBuffer);
//}
//void Graphics::SetVSConstantBuffers(UINT startSlot, UINT NumOfBuffers, ID3D11Buffer* const* ppBuffer)
//{
//	this->deviceContext->VSSetConstantBuffers(startSlot, NumOfBuffers, ppBuffer);
//}
//void Graphics::ClearView(float color[4])
//{
//	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), color);
//}


nvrhi::DeviceHandle Graphics::m_NvrhiDevice;

}