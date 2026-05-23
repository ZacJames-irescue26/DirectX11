#pragma once
#include "BasePass.h"
#include "ShadowPass.h"
#include "GBufferPass.h"


namespace Engine
{

	struct LightingSRVData
	{
		ID3D11ShaderResourceView* IrradianceSRV;
		ID3D11ShaderResourceView* HDRIFrameBufferSRV;
		ID3D11ShaderResourceView* PrefilteringSRV;
		ID3D11ShaderResourceView* BRDFSRV;
		ID3D11ShaderResourceView* DirShadowMapSRV;
		ID3D11ShaderResourceView* CascadeShadowMapSRV[4];

	};

	class LightingPass : public BasePass
	{

	public:
		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

		void Draw(Graphics* gfx, GBufferSRV bufSRV, LightingSRVData SRVData);
		void SetLightMatrixBuffers(ShadowPass& shadow);

		void ImGuiPass() override;


		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;

		private: 
		bool Initialize(ID3D11Device* device) override;
		void Draw(Graphics* gfx) override;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> HDRIsamplerState;
		Microsoft::WRL::ComPtr < ID3D11SamplerState> shadowSampler = nullptr;
		VertexBuffer<FullScreenQuad> m_FullScreenVertex;
		IndexBuffer m_FullScreenIndex;
		VertexShader m_DeferredvertexShader;
		PixelShader m_DeferredpixelShader;
		ConstantBuffer<DirectionalLightParams> m_lightparams;
		ConstantBuffer<CameraInfo> CameraInfoConstantBuffer;
		Microsoft::WRL::ComPtr < ID3D11Texture2D> FinalTexture;
		Microsoft::WRL::ComPtr < ID3D11RenderTargetView> FinalRTV;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> FinalSRV;
	};
}