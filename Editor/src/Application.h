#pragma once
#include <memory>
#include "EngineInclude.h"
#include "Acceleration\BVH\BVH.h"
#include "Passes\BRDFPass.h"
#include "Passes\DrawHDRIPass.h"
#include "Passes\IrradianceConvolutionPass.h"
#include "Passes\PreFIlteringpass.h"
#include "Passes\GBufferPass.h"
#include "Passes\ShadowPass.h"
#include "Passes\LightingPass.h"
#include "ResourcesPanel.h"


using namespace Engine;

class Application : public EngineInit
{
public:
	void Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height);
	void OnCreate();
	void InitializeShaders();
	void OnUpdate();
	void BindGBufferPass();
	void RenderToRaytraceToSRV();
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
	void SpawnSurfels();
	void UpdateSurfels();
	void RayTraceShadows();
	void RenderFrame();
	void ForwardRender();
	void OnImguiRender();
	void OnImguiRenderViewport();
	//Light light;
	//PhysicsObject floor;
	//PhysicsObject gameObject;
	//GameObject helmet;
	//ThirdPersonCamera PlayerCamera;
	//GameObject MiscItems;


	
	std::unique_ptr<Scene> m_Scene;

private:
	bool m_ViewportFocused;
	bool m_ViewportHovered;
	
	VertexShader m_vertexShader;
	PixelShader m_pixelShader;


	IndexBuffer indexBuffer;
	VertexBuffer<Vertex> vertexBuffer;
	
	//Constant Buffers-----------------------------------------------
	
	ConstantBuffer<CB_VS_vertexShader> constantBuffer;
	ConstantBuffer<CB_FS_LightPos> lightConstantBuffer;
	ConstantBuffer<CB_VS_vertexShader> floorConstantBuffer;
	ConstantBuffer<CB_Anim_VS_vertexShader> AnimatedConstantBuffer;
	
	ConstantBuffer<CB_VS_ViewProj> HDRIViewProj;
	


	ConstantBuffer<CB_VS_ViewProj>m_ViewProj;
	ConstantBuffer<DebugColors> m_DebugColors;
	ConstantBuffer<Lights> m_CastLight;
	ConstantBuffer<ShadowlightingInfo> lcb;
	ConstantBuffer<BaseCB> m_BaseCB;
	ConstantBuffer<SurfelCSBUffer> m_SurfelCSBUffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> myTexture;
	data data;
	static bool playercam;
	inline static int windowWidth = 0;
	inline static int windowHeight = 0;

	//GBuffer

	GBufferPass m_GBuffer;
	


	//deferred shader

	LightingPass m_LightingPass;

	

	VertexBuffer<FullScreenQuad> m_FullScreenVertex;
	IndexBuffer m_FullScreenIndex;


	// HDRI
	HDRIPass m_HDRIPass;

	IrradiaceConvolutionPass m_IrradianceConvolution;
	bool RenderIrradianceandHDRI = true;

	//backGround cubemap
	VertexShader m_BackgroundCubemap_VS;
	PixelShader m_BackgroundCubemap_PS;

	//prefiltering
	PrefilteringPass m_Prefiltering;
	//BRDF
	BRDFPass m_BRDFPass;
	//CSM
	ShadowPass m_ShadowPass;

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

	Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > nodeSRV = nullptr;
	Microsoft::WRL::ComPtr < ID3D11Buffer > nodeBuffer = nullptr;
	Microsoft::WRL::ComPtr< ID3D11ShaderResourceView > TrianglesSRV = nullptr;
	Microsoft::WRL::ComPtr < ID3D11Buffer > TrianglesBuffer = nullptr;
	ComputeShader m_Shadow_CS;
	std::unique_ptr<Engine::ModelAccel> accel;
	std::vector<FlatNode> Flat;
	std::vector<Engine::TriangleJustPos> triangles;
	bool ifRaytraceShadows = true;
	bool TakeShot;

	XMMATRIX shadowlightMatrices;
	Microsoft::WRL::ComPtr<ID3D11Buffer> baseRootBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> baseRootSRV;
	std::vector<uint32_t> baseRoots;


	ComputeShader m_GenerateSurfel_CS;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_SurfelsUAV = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_SurfelsBuffer = nullptr;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_SurfelsSRV = nullptr;
	Microsoft::WRL::ComPtr < ID3D11Buffer> surfelCounterBuffer = nullptr;
	Microsoft::WRL::ComPtr < ID3D11UnorderedAccessView> surfelCounterUAV = nullptr;
	Microsoft::WRL::ComPtr < ID3D11Texture2D> m_TileCoverageTex = nullptr;
	Microsoft::WRL::ComPtr < ID3D11UnorderedAccessView> m_TileCoverageUAV = nullptr;
	Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> m_TileCoverageSRV = nullptr;
	bool clearSurfels = false;

	std::unique_ptr<Editor::SceneHierarchyPanel> m_SceneHierarchyPanel;
};