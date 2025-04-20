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
	void BindGBufferPass();
	void BindLightingPass();
	void DrawHDRI();
	void IrradianceConvolution();
	void Prefiltering();
	void BRDF();
	void BackgroundCubeMap();
	void ShadowDepthPass();
	void DrawDebugCascade();
	void DrawShadowMaps();
	void DirectionalShadowMap();
	void DrawSurfels();
	void RenderFrame();
	void ForwardRender();
	void OnUserInput();
	Light light;
	PhysicsObject floor;
	PhysicsObject gameObject;
	GameObject helmet;
	Camera camera;
	ThirdPersonCamera PlayerCamera;
	GameObject MiscItems;
	XMFLOAT3 TargetVec ={0.0,0.0,0.0};
	float shadowDirstance = 257.0;
	float farplane = 250;
	XMMATRIX lightMatrices;

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
	
	//Constant Buffers-----------------------------------------------
	
	ConstantBuffer<CB_VS_vertexShader> constantBuffer;
	ConstantBuffer<CB_FS_LightPos> lightConstantBuffer;
	ConstantBuffer<CB_VS_vertexShader> floorConstantBuffer;
	ConstantBuffer<CameraInfo> CameraInfoConstantBuffer;
	ConstantBuffer<CB_VS_ViewProj> HDRIViewProj;
	ConstantBuffer<PrefilteringParams> m_PrefilteringParams;
	ConstantBuffer<LightSpaceMatrices> m_LightSpace;
	ConstantBuffer<ModelOnly> m_ObjectModel;
	ConstantBuffer<CB_VS_ViewProj>m_ViewProj;
	ConstantBuffer<DebugColors> m_DebugColors;
	ConstantBuffer<Lights> m_CastLight;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;
	data data;
	static bool playercam;
	int windowWidth = 0;
	int windowHeight = 0;

	XMFLOAT2 Sky = {0.2,0.2};
	XMFLOAT3 direction = {0.0,0.0,0.0};
	//GBuffer
	VertexShader m_GBuffervertexShader;
	PixelShader m_GBufferpixelShader;

	D3D11_VIEWPORT viewport;


	//deferred shader

	VertexShader m_DeferredvertexShader;
	PixelShader m_DeferredpixelShader;

	ConstantBuffer<DirectionalLightParams> m_lightparams;

	VertexBuffer<FullScreenQuad> m_FullScreenVertex;
	IndexBuffer m_FullScreenIndex;


	// HDRI
	VertexShader m_EquiToHDRI_VS;
	PixelShader m_EquiToHdri_PS;
	PixelShader m_IrradianceConvolution_PS;
	VertexBuffer<CubeWPos> m_HdriVertex;
	IndexBuffer m_HdriIndex;
	bool RenderIrradianceandHDRI = true;

	//backGround cubemap
	VertexShader m_BackgroundCubemap_VS;
	PixelShader m_BackgroundCubemap_PS;

	//prefiltering
	PixelShader m_Prefiltering_PS;
	//BRDF
	VertexShader m_BRDF_VS;
	PixelShader m_BRDF_PS;
	//CSM
	VertexShader m_ShadowDepth_VS;
	GeometryShader m_ShadowDepth_GS;

	std::vector<VertexBuffer<CubeWPos>> m_DebugCascade;
	IndexBuffer m_CascadeIndex;
	VertexShader m_DebugCascade_VS;
	PixelShader m_DebugCascade_PS;
	PixelShader m_DebugDrawShadowMap_PS;
	int shadowmapIndex = 0;

	// surfels
	SurfelGenerator* gen;
	Octree* octree;

	VertexBuffer<SurfelVB> SurfelVertexBuffer;
	VertexShader m_SurfelDebug_VS;
	PixelShader m_SurfelDebug_PS;
	GeometryShader m_SureflDebug_GS;
	bool drawsurfeldebug = false;
};