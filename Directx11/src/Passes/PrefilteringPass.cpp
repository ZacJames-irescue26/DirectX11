#include "pch.h"
#include "Prefilteringpass.h"
#include "InputElements.h"

namespace Engine
{

	bool PrefilteringPass::Initialize(ID3D11Device* device)
	{
		if (!m_Prefiltering_PS.Initialize(device, L"CompiledShaders/Prefiltering_p.cso"))
		{
			return false;
		}
		if (!m_EquiToHDRI_VS.Initialize(device, L"CompiledShaders/EquiToHdri_v.cso", InputElements::posDesc, ARRAYSIZE(InputElements::posDesc)))
		{
			return false;
		}

		D3D11_SAMPLER_DESC presamplerDesc = {};
		presamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;         // Smooth trilinear filtering
		presamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;           // Clamp to edge to avoid seams
		presamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		presamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		presamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		presamplerDesc.MinLOD = 0;
		presamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
		presamplerDesc.MipLODBias = 0.0f;
		presamplerDesc.MaxAnisotropy = 1;

		COM_ERROR_IF_FAILED(device->CreateSamplerState(&presamplerDesc, &PrefilteredsamplerState), "Failed to create Sampler");


		D3D11_DEPTH_STENCIL_DESC HDRIdepthStencilDesc = {};
		HDRIdepthStencilDesc.DepthEnable = TRUE;
		HDRIdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		HDRIdepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

		COM_ERROR_IF_FAILED(device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled), "Failed to create Stencil");
		
		const UINT baseSize = 128;
		const UINT mipLevels = static_cast<UINT>(std::floor(std::log2(baseSize))) + 1;

		D3D11_TEXTURE2D_DESC pretexDesc = {};
		pretexDesc.Width = baseSize;
		pretexDesc.Height = baseSize;
		pretexDesc.MipLevels = mipLevels;
		pretexDesc.ArraySize = 6; // Cube has 6 faces
		pretexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		pretexDesc.SampleDesc.Count = 1;
		pretexDesc.Usage = D3D11_USAGE_DEFAULT;
		pretexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		pretexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		device->CreateTexture2D(&pretexDesc, nullptr, &PrefilteringTexture);

		COM_ERROR_IF_FAILED(device->CreateTexture2D(&pretexDesc, nullptr, PrefilteringTexture.GetAddressOf()), "Failed to create texture");

		PrefilteringRTVs.resize(mipLevels);
		for (int i = 0; i < PrefilteringRTVs.size(); i++)
		{
			PrefilteringRTVs[i].resize(6);
		}

		for (UINT mip = 0; mip < mipLevels; ++mip)
		{
			for (UINT face = 0; face < 6; ++face)
			{
				D3D11_RENDER_TARGET_VIEW_DESC prertvDesc = {};
				prertvDesc.Format = pretexDesc.Format;
				prertvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
				prertvDesc.Texture2DArray.MipSlice = mip;
				prertvDesc.Texture2DArray.FirstArraySlice = face;
				prertvDesc.Texture2DArray.ArraySize = 1;

				COM_ERROR_IF_FAILED(device->CreateRenderTargetView(PrefilteringTexture.Get(), &prertvDesc, PrefilteringRTVs[mip][face].GetAddressOf()), "Failed to create RTV");
			}
		}
		D3D11_SHADER_RESOURCE_VIEW_DESC presrvDesc = {};
		presrvDesc.Format = pretexDesc.Format;
		presrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		presrvDesc.TextureCube.MipLevels = mipLevels;
		presrvDesc.TextureCube.MostDetailedMip = 0;

		device->CreateShaderResourceView(PrefilteringTexture.Get(), &presrvDesc, &PrefilteringSRV);

		//------------------------------Cube--------------------------------------------------//
		std::vector<CubeWPos> cubevertices = {

			// Front face
			{ XMFLOAT3(-1.0f,  1.0f, -1.0f) },
			{ XMFLOAT3(1.0f,  1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },

			// Back face
			{ XMFLOAT3(1.0f,  1.0f,  1.0f) },
			{ XMFLOAT3(-1.0f,  1.0f,  1.0f) },
			{ XMFLOAT3(1.0f, -1.0f,  1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f,  1.0f) },

			// Left face
			{ XMFLOAT3(-1.0f,  1.0f,  1.0f) },
			{ XMFLOAT3(-1.0f,  1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f,  1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },

			// Right face
			{ XMFLOAT3(1.0f,  1.0f, -1.0f) },
			{ XMFLOAT3(1.0f,  1.0f,  1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, -1.0f,  1.0f) },

			// Top face
			{ XMFLOAT3(-1.0f,  1.0f,  1.0f) },
			{ XMFLOAT3(1.0f,  1.0f,  1.0f) },
			{ XMFLOAT3(-1.0f,  1.0f, -1.0f) },
			{ XMFLOAT3(1.0f,  1.0f, -1.0f) },

			// Bottom face
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f,  1.0f) },
			{ XMFLOAT3(1.0f, -1.0f,  1.0f) }
		};


		COM_ERROR_IF_FAILED(m_HdriVertex.Initialize(device, cubevertices.data(), cubevertices.size()), "Failed to initialize Vertex buffer.");
		DWORD cubeindices[] = {
			// Front face
				0, 1, 2,
				2, 1, 3,

				// Back face
				4, 5, 6,
				6, 5, 7,

				// Left face
				8, 9, 10,
				10, 9, 11,

				// Right face
				12, 13, 14,
				14, 13, 15,

				// Top face
				16, 17, 18,
				18, 17, 19,

				// Bottom face
				20, 21, 22,
				22, 21, 23
		};

		COM_ERROR_IF_FAILED(m_HdriIndex.Initialize(device, cubeindices, 36), "Failed to initialize index buffer.");

		return true;
	}

	bool PrefilteringPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Initialize(device);
		COM_ERROR_IF_FAILED(m_PrefilteringParams.Initialize(device, deviceContext), "Failed to initialize constant buffer.");
		COM_ERROR_IF_FAILED(HDRIViewProj.Initialize(device, deviceContext), "Failed to create buffer");
	
		return true;
	}

	void PrefilteringPass::Draw(Graphics* gfx)
	{
		
		gfx->GetDeviceContext()->PSSetSamplers(0, 1, PrefilteredsamplerState.GetAddressOf());
		gfx->GetDeviceContext()->OMSetDepthStencilState(HDRIdepthStencilStateDisabled.Get(), 0);

		gfx->SetInputLayout(this->m_EquiToHDRI_VS.GetInputLayout());
		gfx->SetRasterizerState();
		gfx->SetBlendState();
		gfx->GetDeviceContext()->VSSetShader(m_EquiToHDRI_VS.GetShader(), NULL, 0);
		gfx->GetDeviceContext()->PSSetShader(m_Prefiltering_PS.GetShader(), NULL, 0);

		
	}

	void PrefilteringPass::Draw(Graphics* gfx, ID3D11ShaderResourceView** HDRISRV)
	{
		Draw(gfx);
		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		const UINT baseSize = 128;
		const UINT mipLevels = static_cast<UINT>(std::floor(std::log2(baseSize))) + 1;
		UINT baseResolution = 128;
		for (UINT mip = 0; mip < mipLevels; ++mip)
		{
			UINT mipWidth = static_cast<UINT>(baseResolution * std::pow(0.5f, mip));
			UINT mipHeight = static_cast<UINT>(baseResolution * std::pow(0.5f, mip));

			// Resize viewport
			D3D11_VIEWPORT previewport = {};
			previewport.TopLeftX = 0;
			previewport.TopLeftY = 0;
			previewport.Width = mipWidth;
			previewport.Height = mipHeight;
			previewport.MinDepth = 0.0f;
			previewport.MaxDepth = 1.0f;
			gfx->GetDeviceContext()->RSSetViewports(1, &previewport);
			gfx->GetDeviceContext()->PSSetShaderResources(0, 1, HDRISRV);
			gfx->GetDeviceContext()->PSSetConstantBuffers(0, 1, m_PrefilteringParams.GetAddressOf());
			float roughness = (float)mip / (float)(mipLevels - 1);
			m_PrefilteringParams.data.roughness = roughness;
			m_PrefilteringParams.ApplyChanges();




			for (UINT face = 0; face < 6; ++face)
			{
				XMMATRIX views[6] = {
					XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(1, 0, 0, 0), XMVectorSet(0, 1, 0, 0)), // +X
					XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0)), // -X
					XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 0, -1, 0)), // +Y
					XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0)), // -Y
					XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 1, 0, 0)),  // +Z
					XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 0, -1, 0), XMVectorSet(0, 1, 0, 0)), // -Z
				};

				XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 10.0f);
				// Set view matrix for that face
				HDRIViewProj.data.View = XMMatrixTranspose(views[face]);
				HDRIViewProj.data.Projection = XMMatrixTranspose(proj);
				HDRIViewProj.ApplyChanges();
				gfx->GetDeviceContext()->VSSetConstantBuffers(0, 1, HDRIViewProj.GetAddressOf());

				// Bind the correct mip level and face


				gfx->GetDeviceContext()->OMSetRenderTargets(1, PrefilteringRTVs[mip][face].GetAddressOf(), nullptr);
				gfx->GetDeviceContext()->ClearRenderTargetView(PrefilteringRTVs[mip][face].Get(), bgcolor);


				gfx->GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
				gfx->GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

				gfx->GetDeviceContext()->DrawIndexed(36, 0, 0);
			}
		}
	}

	void PrefilteringPass::ImGuiPass()
	{
		
	}

	std::vector<ID3D11ShaderResourceView*> PrefilteringPass::GetSRVRenderTarget()
	{
		return { PrefilteringSRV.Get() };
	}

}