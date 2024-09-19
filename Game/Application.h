#pragma once
#include <memory>
#include "EngineInclude.h"

using namespace Engine;
class Application : public EngineInit
{
public:
	void Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	void OnCreate();
	void InitializeShaders();
	void OnUpdate();
	void RenderFrame();
	void OnUserInput();
	Light light;
	PhysicsObject floor;
	PhysicsObject gameObject;
	GameObject object;
	Camera camera;
	ThirdPersonCamera PlayerCamera;

private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerstate;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthStencilState;
	VertexShader m_vertexShader;
	PixelShader m_pixelShader;


	IndexBuffer indexBuffer;
	VertexBuffer<Vertex> vertexBuffer;
	ConstantBuffer<CB_VS_vertexShader> constantBuffer;
	ConstantBuffer<CB_FS_LightPos> lightConstantBuffer;
	ConstantBuffer<CB_VS_vertexShader> floorConstantBuffer;


	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;
	data data;
	static bool playercam;
	int windowWidth = 0;
	int windowHeight = 0;
	
};