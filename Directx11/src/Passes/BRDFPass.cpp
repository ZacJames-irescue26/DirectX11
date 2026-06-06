#include "pch.h"
#include "BRDFPass.h"
#include "InputElements.h"


namespace Engine {


	bool BRDFPass::Initialize(ID3D11Device* device)
	{
		D3D11_TEXTURE2D_DESC BRDFDesc = {};
		BRDFDesc.Width = 512;
		BRDFDesc.Height = 512;
		BRDFDesc.MipLevels = 1;
		BRDFDesc.ArraySize = 1;
		BRDFDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		BRDFDesc.SampleDesc.Count = 1;
		BRDFDesc.Usage = D3D11_USAGE_DEFAULT;
		BRDFDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;


		COM_ERROR_IF_FAILED(device->CreateTexture2D(&BRDFDesc, nullptr, BRDFTexture.GetAddressOf()), "Failed to create texture");

		D3D11_RENDER_TARGET_VIEW_DESC BRDFrtvDesc = {};
		BRDFrtvDesc.Format = BRDFDesc.Format;
		BRDFrtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		BRDFrtvDesc.Texture2DArray.MipSlice = 0;
		BRDFrtvDesc.Texture2DArray.ArraySize = 1;

		COM_ERROR_IF_FAILED(device->CreateRenderTargetView(BRDFTexture.Get(), &BRDFrtvDesc, &BRDFRTVs), "Failed to create RTV");


		D3D11_SHADER_RESOURCE_VIEW_DESC BRDFsrvDesc = {};
		BRDFsrvDesc.Format = BRDFDesc.Format;
		BRDFsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		BRDFsrvDesc.Texture2D.MipLevels = 1;




		COM_ERROR_IF_FAILED(device->CreateShaderResourceView(BRDFTexture.Get(), &BRDFsrvDesc, &BRDFSRV), "Failed to Create SRV");
		if (!m_BRDF_VS.Initialize(device, (LPCWSTR)Project::GetEditorShaderPath(L"BRDF_v.cso").wstring().c_str(), InputElements::FullScreenRectlayout, ARRAYSIZE(InputElements::FullScreenRectlayout)))
		{
			return false;
		}
		if (!m_BRDF_PS.Initialize(device, (LPCWSTR)Project::GetEditorShaderPath(L"BRDF_p.cso").wstring().c_str()))
		{
			return false;
		}

		std::vector<FullScreenQuad> vertices = {
			// Positions (x, y, z) and Texture coordinates (u, v)
			{XMFLOAT2(-1.0f,  1.0f),XMFLOAT2(0.0f, 0.0f)}, // Top-left
			{XMFLOAT2(1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) }, // Top-right
			{XMFLOAT2(-1.0f, -1.0f),XMFLOAT2(0.0f, 1.0f) }, // Bottom-left
			{XMFLOAT2(1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) }, // Bottom-right
		};

		COM_ERROR_IF_FAILED(m_FullScreenVertex.Initialize(device, vertices.data(), vertices.size()), "Failed to create vertex buffer");

		DWORD indices[] = {
			0, 1, 2, // First triangle
			2, 1, 3  // Second triangle
		};
		COM_ERROR_IF_FAILED(m_FullScreenIndex.Initialize(device, indices, 6) , "Failed to initialize index buffer.");
		return true;
	}

	void BRDFPass::Draw(Graphics* gfx)
	{
		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		// Resize viewport
		D3D11_VIEWPORT BRDFviewport = {};
		BRDFviewport.TopLeftX = 0;
		BRDFviewport.TopLeftY = 0;
		BRDFviewport.Width = 512;
		BRDFviewport.Height = 512;
		BRDFviewport.MinDepth = 0.0f;
		BRDFviewport.MaxDepth = 1.0f;
		gfx->GetDeviceContext()->OMSetRenderTargets(1, BRDFRTVs.GetAddressOf(), nullptr);

		gfx->GetDeviceContext()->RSSetViewports(1, &BRDFviewport);
		gfx->SetRasterizerState();
		gfx->SetBlendState();
		gfx->GetDeviceContext()->OMSetDepthStencilState(gfx->depthStencilStateDisabled.Get(), 0);
		gfx->SetInputLayout(m_BRDF_VS.GetInputLayout());

		gfx->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfx->GetDeviceContext()->VSSetShader(m_BRDF_VS.GetShader(), NULL, 0);
		gfx->GetDeviceContext()->PSSetShader(m_BRDF_PS.GetShader(), NULL, 0);
		UINT offset = 0;
		gfx->GetDeviceContext()->IASetVertexBuffers(0, 1, m_FullScreenVertex.GetAddressOf(), m_FullScreenVertex.StridePtr(), &offset);
		gfx->GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

		gfx->GetDeviceContext()->DrawIndexed(6, 0, 0);
	}

	void BRDFPass::ImGuiPass()
	{
		
	}

	std::vector<ID3D11ShaderResourceView*> BRDFPass::GetSRVRenderTarget()
	{
		return { BRDFSRV.Get()};
	}

}