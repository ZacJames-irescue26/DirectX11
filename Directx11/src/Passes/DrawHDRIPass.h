#pragma once
#include "BasePass.h"

namespace Engine
{
	class HDRIPass : public BasePass
	{

	public:
		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext);

		void Draw(Graphics* gfx) override;


		void ImGuiPass() override;

		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;

	private:
		bool Initialize(ID3D11Device* device) override;
		VertexShader m_EquiToHDRI_VS;
		PixelShader m_EquiToHdri_PS;
		Microsoft::WRL::ComPtr<ID3D11SamplerState> HDRIsamplerState;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilState> HDRIdepthStencilStateDisabled;
		Microsoft::WRL::ComPtr < ID3D11Texture2D> HDRITexture;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> HDRISRV;
		ConstantBuffer<CB_VS_ViewProj> HDRIViewProj;
		Microsoft::WRL::ComPtr < ID3D11Texture2D> HDRIFramebufferTexture;
		Microsoft::WRL::ComPtr <ID3D11RenderTargetView> HDRIFramebufferRTV[6];
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> HDRIFramebufferSRV;
		VertexBuffer<CubeWPos> m_HdriVertex;
		IndexBuffer m_HdriIndex;
		std::string HDRIFilepath = "Assets/HDRI/kloppenheim_06_puresky_4k.hdr";
	};
}