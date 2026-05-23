#pragma once
#include "BasePass.h"
#include "Scene\Scene.h"


namespace Engine
{
	struct GBufferSRV
	{
		GBufferSRV(ID3D11ShaderResourceView* normal, ID3D11ShaderResourceView* diffuse, ID3D11ShaderResourceView* specular, ID3D11ShaderResourceView* position, ID3D11ShaderResourceView* Depth)
		: m_normal(normal), m_diffuse(diffuse), m_Specular(specular), m_position(position), m_Depth(Depth)
		{ }
		ID3D11ShaderResourceView* m_normal;
		ID3D11ShaderResourceView* m_diffuse;
		ID3D11ShaderResourceView* m_Specular;
		ID3D11ShaderResourceView* m_position;
		ID3D11ShaderResourceView* m_Depth;
	};
	class GBufferPass : public BasePass
	{
	public:


		bool Initialize(ID3D11Device* device) override;

		void Draw(Graphics* gfx, Scene& scene);

		GBufferSRV GetGBufferSRV()
		{
			GBufferSRV gbuf(NormalSRV.Get(), DiffuseSRV.Get(), SpecularSRV.Get(), positionSRV.Get(), DepthSRV.Get());
			return gbuf;
		}

		void ImGuiPass() override;

		std::vector<ID3D11ShaderResourceView*> GetSRVRenderTarget() override;

	private:
		void Draw(Graphics* gfx) override;

		VertexShader m_GBuffervertexShader;
		PixelShader m_GBufferpixelShader;

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

		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> depthStencilView;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;
		Microsoft::WRL::ComPtr < ID3D11ShaderResourceView> DepthSRV;
	};

}