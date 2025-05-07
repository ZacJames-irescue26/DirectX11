#pragma once
#include "AdapterReader.h"
#include "Shader.h"
#include "ConstantBufferTypes.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "Game/GameObject.h"
#include "Light.h"
#include "Physics/PhysicsWorld.h"

#include "nvrhi/d3d12.h"
#include "nvrhi/validation.h"

namespace Engine
{

	struct DefaultMessageCallback : public nvrhi::IMessageCallback
	{
		static DefaultMessageCallback& GetInstance();

		void message(nvrhi::MessageSeverity severity, const char* messageText) override;
	};
	struct InstanceParameters
	{
		bool enableWarningsAsErrors = false;
		bool enableGPUValidation = false; // Affects only DX12
		bool headlessDevice = false;
#if  AFTERMATH
		bool enableAftermath = true;
#endif
		bool logBufferLifetime = false;
		bool enableHeapDirectlyIndexed = false; // Allows ResourceDescriptorHeap on DX12




	};


	struct AdapterInfo
	{
		typedef std::array<uint8_t, 16> UUID;
		typedef std::array<uint8_t, 8> LUID;

		std::string name;
		uint32_t vendorID = 0;
		uint32_t deviceID = 0;
		uint64_t dedicatedVideoMemory = 0;

		std::optional<UUID> uuid;
		std::optional<LUID> luid;

		nvrhi::RefCountPtr<IDXGIAdapter> dxgiAdapter;
	};
	struct DeviceCreationParameters : public InstanceParameters
	{
		bool startMaximized = false; // ignores backbuffer width/height to be monitor size
		bool startFullscreen = false;
		bool startBorderless = false;
		bool allowModeSwitch = false;
		int windowPosX = -1;            // -1 means use default placement
		int windowPosY = -1;
		uint32_t backBufferWidth = 1920;
		uint32_t backBufferHeight = 1080;
		uint32_t refreshRate = 0;
		uint32_t swapChainBufferCount = 3;
		nvrhi::Format swapChainFormat = nvrhi::Format::SRGBA8_UNORM;
		uint32_t swapChainSampleCount = 1;
		uint32_t swapChainSampleQuality = 0;
		uint32_t maxFramesInFlight = 2;
		bool enableNvrhiValidationLayer = false;
		bool vsyncEnabled = false;
		bool enableComputeQueue = false;
		bool enableCopyQueue = false;

		// Index of the adapter (DX11, DX12) or physical device (Vk) on which to initialize the device.
		// Negative values mean automatic detection.
		// The order of indices matches that returned by DeviceManager::EnumerateAdapters.
		int adapterIndex = -1;

		// Set this to true if the application implements UI scaling for DPI explicitly instead of relying
		// on ImGUI's DisplayFramebufferScale. This produces crisp text and lines at any scale
		// but requires considerable changes to applications that rely on the old behavior:
		// all UI sizes and offsets need to be computed as multiples of some scaled parameter,
		// such as ImGui::GetFontSize(). Note that the ImGUI style is automatically reset and scaled in 
		// ImGui_Renderer::DisplayScaleChanged(...).
		//
		// See ImGUI FAQ for more info:
		//   https://github.com/ocornut/imgui/blob/master/docs/FAQ.md#q-how-should-i-handle-dpi-in-my-application
		bool supportExplicitDisplayScaling = false;

		// Enables automatic resizing of the application window according to the DPI scaling of the monitor
		// that it is located on. When set to true and the app launches on a monitor with >100% scale, 
		// the initial window size will be larger than specified in 'backBufferWidth' and 'backBufferHeight' parameters.
		bool resizeWindowWithDisplayScale = false;

		nvrhi::IMessageCallback* messageCallback = nullptr;
		DXGI_USAGE swapChainUsage = DXGI_USAGE_SHADER_INPUT | DXGI_USAGE_RENDER_TARGET_OUTPUT;
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;



	};
class Graphics
{

public:

	~Graphics();
	void DestroyDeviceAndSwapChain();
	void ReleaseRenderTargets();
	void ReportLiveObjects();
	bool CreateInstanceInternal();
	bool EnumerateAdapters(std::vector<AdapterInfo>& outAdapters);
	bool CreateDevice();
	bool CreateSwapChain(HWND hwnd);
	bool CreateRenderTargets();
	void ResizeSwapChain();
	bool BeginFrame();
	void BackBufferResized();
	nvrhi::ITexture* GetBackBuffer(uint32_t index);
	void BackBufferResizing();
	uint32_t GetCurrentBackBufferIndex();
	uint32_t GetBackBufferCount();
	bool Initialize(HWND hwnd, int width, int height);
	void PhysicsUpdate();
	bool Present();
	void RenderFrame();
	Camera camera;
	void ClearDepthStencil(ID3D11DepthStencilView* stencil);
	void SetInputLayout(ID3D11InputLayout* layout);
	void SetTopology(D3D11_PRIMITIVE_TOPOLOGY top);
	void SetRasterizerState();
	void SetDepthStencilState();
	void SetBlendState();
	void SetSamplers();
	void SetPSShader(ID3D11PixelShader* shader);
	void SetVSShader(ID3D11VertexShader* shader);
	void SetPSConstantBuffers(UINT startSlot, UINT NumOfBuffers, ID3D11Buffer* const* ppBuffer);
	void SetVSConstantBuffers(UINT startSlot, UINT NumOfBuffers, ID3D11Buffer* const* ppBuffer);
	void ClearView(float color[4]);
	nvrhi::DeviceHandle                             m_NvrhiDevice;
private:
	Light light;
	PhysicsObject floor;
	PhysicsObject gameObject;
	PhysicsEngine physicsController;
	nvrhi::RefCountPtr<IDXGIFactory2>               m_DxgiFactory2;
	nvrhi::RefCountPtr<ID3D12Device>                m_Device12;
	nvrhi::RefCountPtr<ID3D12CommandQueue>          m_GraphicsQueue;
	nvrhi::RefCountPtr<ID3D12CommandQueue>          m_ComputeQueue;
	nvrhi::RefCountPtr<ID3D12CommandQueue>          m_CopyQueue;
	nvrhi::RefCountPtr<IDXGISwapChain3>             m_SwapChain;
	DXGI_SWAP_CHAIN_DESC1                           m_SwapChainDesc{};
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC                 m_FullScreenDesc{};
	nvrhi::RefCountPtr<IDXGIAdapter>                m_DxgiAdapter;
	HWND                                            m_hWnd = nullptr;
	bool                                            m_TearingSupported = false;

	std::vector<nvrhi::RefCountPtr<ID3D12Resource>> m_SwapChainBuffers;
	std::vector<nvrhi::TextureHandle>               m_RhiSwapChainBuffers;
	nvrhi::RefCountPtr<ID3D12Fence>                 m_FrameFence;
	std::vector<HANDLE>                             m_FrameFenceEvents;

	UINT64                                          m_FrameCount = 1;



	std::string                                     m_RendererString;
	
	int windowWidth = 0;
	int windowHeight = 0;

	bool enableGPUValidation = false;
	bool enableDebugRuntime = _DEBUG;
	DeviceCreationParameters m_DeviceParams;
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();

	std::string GetAdapterName(DXGI_ADAPTER_DESC const& aDesc)
	{
		size_t length = wcsnlen(aDesc.Description, _countof(aDesc.Description));

		std::string name;
		name.resize(length);
		WideCharToMultiByte(CP_ACP, 0, aDesc.Description, int(length), name.data(), int(name.size()), nullptr, nullptr);

		return name;
	}


	std::vector<XMFLOAT3> GetFrustumCornersWorldSpace(const XMMATRIX& viewProj);
	std::vector<XMFLOAT3> GetFrustumCornersWorldSpaceViewProj(const XMMATRIX& projView);
	std::vector<XMFLOAT3> getFrustumCornersWorldSpace(const XMMATRIX& proj, const XMMATRIX& view);
	XMMATRIX getLightSpaceMatrix(const float nearPlane, const float farPlane);
	VertexShader m_vertexShader;
	PixelShader m_pixelShader;
	
	
	IndexBuffer indexBuffer;
	VertexBuffer<Vertex> vertexBuffer;
	ConstantBuffer<CB_VS_vertexShader> constantBuffer;
	ConstantBuffer<CB_FS_LightPos> lightConstantBuffer;
	ConstantBuffer<CB_VS_vertexShader> floorConstantBuffer;
public:

	nvrhi::CommandListHandle m_CommandList;

	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> HDRIsamplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> PrefilteredsamplerState;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;

	nvrhi::TextureHandle DepthTexture;

	nvrhi::TextureHandle positionTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> positionRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> positionSRV;

	nvrhi::TextureHandle NormalTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView>  NormalRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView>  NormalSRV;

	nvrhi::TextureHandle DiffuseTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> DiffuseRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> DiffuseSRV;

	nvrhi::TextureHandle SpecularTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> SpecularRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> SpecularSRV;
	nvrhi::FramebufferHandle GBUfferFrameBuffer;

	//HDRI
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilStateDisabled;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> HDRIdepthStencilStateDisabled;
	Microsoft::WRL::ComPtr < ID3D11Texture2D> HDRITexture;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> HDRISRV;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> HDRIFramebufferTexture;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> HDRIFramebufferRTV[6];
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> HDRIFramebufferSRV;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> IrradiancemapTexture;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> IrradianceMapSRV;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> irradianceRTVs[6];

	Microsoft::WRL::ComPtr<ID3D11Texture2D> irradiancedepthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> irradiancedepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> IrradiancedepthStencilStateDisabled;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilSkyboxState;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> PrefilteringTexture;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> PrefilteringSRV;
	std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>> PrefilteringRTVs;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> BRDFTexture;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> BRDFSRV;
	Microsoft::WRL::ComPtr <ID3D11RenderTargetView> BRDFRTVs;
	//CSM -----------------------------------------------------------------------------
	const int NUM_CASCADES = 4;
	std::vector<float> shadowCascadeLevels{ 1000 / 50.0f, 1000 / 25.0f, 1000 / 10.0f, 1000 / 2.0f };
	const unsigned int depthMapResolution = 4098;
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> shadowDSVs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> shadowSRVs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> shadowTex;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> shadowDepthStencilState;

	XMFLOAT3 LightDir;
	std::vector<XMMATRIX> getLightSpaceMatrices();

	std::vector<XMVECTOR> getFrustumCornersWorldSpace(const XMMATRIX& projview);

	Microsoft::WRL::ComPtr<ID3D12BlendState> transparentBlendState;
	Microsoft::WRL::ComPtr<ID3D12RasterizerState> DebugLineState;
	Microsoft::WRL::ComPtr<ID3D12Texture2D> ShadowtextureArray;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowtextureArraySRV;
	Microsoft::WRL::ComPtr < ID3D11SamplerState> shadowSampler = nullptr;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DirectionalshadowDSVs;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DirectionalshadowSRVs;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> DirectionalshadowTex;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterState;

	//OPTIX TEXTURE
	Microsoft::WRL::ComPtr<ID3D11Texture2D> irrTexDX;

	static Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> irrSRV;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> irratlasTex;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> irratlasSRV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> irrUAV;
};
}