#pragma once
#include "BasePass.h"
#include "Scene\Scene.h"

namespace Engine
{

	struct CascadeSRV
	{
		
		std::vector<ID3D11ShaderResourceView**> Cascades;
	};

	class ShadowPass : public BasePass
	{
	public:

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

		
		ID3D11ShaderResourceView* GetDirSRV()
		{
			return DirectionalshadowSRVs.Get();
		}
		ID3D11ShaderResourceView* GetCascades(uint32_t level) 
		{
			return shadowSRVs[level].Get();
		}
		std::vector<XMMATRIX> GetCascadeMatrix() const 
		{
			return m_CascadeLightVP;
		}
		std::vector<float> GetCascadesLevels() const
		{
			return shadowCascadeLevels;
		}
		XMMATRIX GetDirShadowMatrix() const 
		{
			return lightMatrices;
		}
		XMFLOAT3 GetLightDir()
		{
			return direction;
		}
		
		void CalcCascadeOrthoProjs(Graphics* gfx);
		void ImGuiPass() override;

		void Draw(Graphics* gfx, Scene& scene);

		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;

		private:
		bool Initialize(ID3D11Device* device) override;
		const int NUM_CASCADES = 4;
		std::vector<float> shadowCascadeLevels{ 0.01, 1, 3, 8, 10, 20 };
		//std::vector<float> shadowCascadeLevels{ 0.1, 10, 30, 60, 80, 100 };
		const unsigned int depthMapResolution = 2048;

		void Draw(Graphics* gfx) override;

		XMFLOAT2 Sky = { 0.2,0.2 };
		XMFLOAT3 direction = { 0.0,-1.0,0.0 };
		float shadowDirstance = 257.0;
		float farplane = 100;
		XMFLOAT3 TargetVec = { 0.0,0.0,0.0 };
		std::vector<XMMATRIX> m_CascadeLightVP;
		XMMATRIX lightMatrices;
		std::vector<Microsoft::WRL::ComPtr<ID3D11DepthStencilView>> shadowDSVs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> shadowSRVs;
		std::vector<Microsoft::WRL::ComPtr<ID3D11Texture2D>> shadowTex;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> shadowDepthStencilState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DirectionalshadowDSVs;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> DirectionalshadowSRVs;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> DirectionalshadowTex;
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterState;
		Microsoft::WRL::ComPtr < ID3D11SamplerState> shadowSampler = nullptr;

		VertexShader m_ShadowDepth_VS;
		GeometryShader m_ShadowDepth_GS;
		ConstantBuffer<ModelOnly> m_ObjectModel;
		ConstantBuffer<LightSpaceMatrices> m_LightSpace;
	};
}