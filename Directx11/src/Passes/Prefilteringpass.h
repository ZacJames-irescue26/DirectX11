#pragma once
#include "BasePass.h"

namespace Engine
{
	class PrefilteringPass : public BasePass
	{
	public:

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

		void Draw(Graphics* gfx, ID3D11ShaderResourceView** HDRISRV);

		void ImGuiPass() override;


		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;
	private:
		void Draw(Graphics* gfx) override;
		bool Initialize(ID3D11Device* device) override;

		VertexShader m_EquiToHDRI_VS;
		PixelShader m_Prefiltering_PS;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> PrefilteredsamplerState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> HDRIdepthStencilStateDisabled;
		ConstantBuffer<PrefilteringParams> m_PrefilteringParams;
		ConstantBuffer<CB_VS_ViewProj> HDRIViewProj;
		std::vector<std::vector<Microsoft::WRL::ComPtr<ID3D11RenderTargetView>>> PrefilteringRTVs;
		Microsoft::WRL::ComPtr < ID3D11Texture2D> PrefilteringTexture;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> PrefilteringSRV;
		VertexBuffer<CubeWPos> m_HdriVertex;
		IndexBuffer m_HdriIndex;


	};
}