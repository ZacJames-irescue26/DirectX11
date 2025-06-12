#pragma once
#include "AdapterReader.h"
#include "Shader.h"
#include "ConstantBufferTypes.h"
#include "ConstantBuffer.h"
#include "Camera.h"
#include "Game/GameObject.h"
#include "Light.h"
#include "Physics/PhysicsWorld.h"

namespace Engine
{
class Graphics
{

public:

	~Graphics();
	bool Initialize(HWND hwnd, int width, int height);
	void PhysicsUpdate();
	void Present();
	void RenderFrame();
	Camera camera;
	static ID3D11Device* GetDevice()
	{
		return device.Get();
	}
	ID3D11DeviceContext* GetDeviceContext()
	{
		return deviceContext.Get();
	}
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

	
	Light light;
	PhysicsObject floor;
	PhysicsObject gameObject;
	PhysicsEngine physicsController;
	static Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain> swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerstate;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DepthBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	int windowWidth = 0;
	int windowHeight = 0;
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();




	
	//std::vector<XMFLOAT3> GetFrustumCornersWorldSpace(const XMMATRIX& viewProj);
	//std::vector<XMFLOAT3> GetFrustumCornersWorldSpaceViewProj(const XMMATRIX& projView);
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
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> HDRIsamplerState;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> PrefilteredsamplerState;
	//Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;


	Microsoft::WRL::ComPtr < ID3D11Texture2D> positionTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> positionRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> positionSRV;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> NormalTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView>  NormalRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView>  NormalSRV;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> DiffuseTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> DiffuseRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> DiffuseSRV;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> SpecularTexture;
	Microsoft::WRL::ComPtr < ID3D11RenderTargetView> SpecularRTV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> SpecularSRV;

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
	std::vector<float> shadowCascadeLevels{ 0.01, 0.1, 1, 5, 10, 100 };
	const unsigned int depthMapResolution = 2048;
	const int DownSampleMultiplier = 4;
	std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> shadowDSVs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> shadowSRVs;
	std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> shadowTex;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> shadowDepthStencilState;

	XMFLOAT3 LightDir;
	std::vector<XMMATRIX> getLightSpaceMatrices();

	std::vector<XMVECTOR> getFrustumCornersWorldSpace(const XMMATRIX& projview);

	Microsoft::WRL::ComPtr<ID3D11BlendState> transparentBlendState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> DebugLineState;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ShadowtextureArray;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShadowtextureArraySRV;
	Microsoft::WRL::ComPtr < ID3D11SamplerState> shadowSampler = nullptr;

	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DirectionalshadowDSVs;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DirectionalshadowSRVs;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> DirectionalshadowTex;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterState;

	Microsoft::WRL::ComPtr < ID3D11Texture2D> RaytracedshadowTex;
	Microsoft::WRL::ComPtr < ID3D11UnorderedAccessView> shadowUAV;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> RaytracedShadowSRV;

	std::vector<XMMATRIX> m_CascadeLightVP;
	void CalcCascadeOrthoProjs();
	XMFLOAT2 Sky = { 0.2,0.2 };
	XMFLOAT3 direction = { 0.0,-1.0,0.0 };
	float shadowDirstance = 257.0;

};
}