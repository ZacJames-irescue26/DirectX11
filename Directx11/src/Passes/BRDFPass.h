#pragma once
#include "BasePass.h"



namespace Engine
{

	class BRDFPass : public BasePass
	{
	public:

		bool Initialize(ID3D11Device* device) override;


		void Draw(Graphics* gfx) override;

		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;

		void ImGuiPass() override;
	private:
		Microsoft::WRL::ComPtr <ID3D11RenderTargetView> BRDFRTVs;
		Microsoft::WRL::ComPtr < ID3D11Texture2D> BRDFTexture;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> BRDFSRV;
		VertexShader m_BRDF_VS;
		PixelShader m_BRDF_PS;
		VertexBuffer<FullScreenQuad> m_FullScreenVertex;
		IndexBuffer m_FullScreenIndex;


	};

}