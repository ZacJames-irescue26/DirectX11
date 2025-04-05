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
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	int windowWidth = 0;
	int windowHeight = 0;
private:
	bool InitializeDirectX(HWND hwnd);
	bool InitializeShaders();
	bool InitializeScene();




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
};
}