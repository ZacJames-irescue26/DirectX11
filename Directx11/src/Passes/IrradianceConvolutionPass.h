#pragma once
#include "BasePass.h"

namespace Engine
{
	class IrradiaceConvolutionPass : public BasePass
	{

	public:

		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
		void Draw(Graphics* gfx, ID3D11ShaderResourceView* HDRISRV);


		void ImGuiPass() override;


		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;

	private:
		void Draw(Graphics* gfx) override;
		bool Initialize(ID3D11Device* device) override;
		VertexShader m_EquiToHDRI_VS;
		PixelShader m_IrradianceConvolution_PS;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> HDRIsamplerState;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> irradiancedepthStencilBuffer;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> irradiancedepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> IrradiancedepthStencilStateDisabled;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> HDRIdepthStencilStateDisabled;
		Microsoft::WRL::ComPtr < ID3D11Texture2D> IrradiancemapTexture;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> IrradianceMapSRV;
		Microsoft::WRL::ComPtr <ID3D11RenderTargetView> irradianceRTVs[6];
		VertexBuffer<CubeWPos> m_HdriVertex;
		IndexBuffer m_HdriIndex;

		ConstantBuffer<CB_VS_ViewProj> HDRIViewProj;


	};
}