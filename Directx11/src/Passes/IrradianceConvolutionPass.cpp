#include "pch.h"
#include "IrradianceConvolutionPass.h"
#include "InputElements.h"

namespace Engine
{

	bool IrradiaceConvolutionPass::Initialize(ID3D11Device* device)
	{
		D3D11_SAMPLER_DESC HDRIsampDesc = {};
		HDRIsampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		HDRIsampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		HDRIsampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		HDRIsampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		COM_ERROR_IF_FAILED(device->CreateSamplerState(&HDRIsampDesc, &HDRIsamplerState), "Failed to make hdri sampler");
	

		D3D11_TEXTURE2D_DESC irradiancedepthDesc = {};
		irradiancedepthDesc.Width = 32;
		irradiancedepthDesc.Height = 32;
		irradiancedepthDesc.MipLevels = 1;
		irradiancedepthDesc.ArraySize = 1;
		irradiancedepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		irradiancedepthDesc.SampleDesc.Count = 1;
		irradiancedepthDesc.Usage = D3D11_USAGE_DEFAULT;
		irradiancedepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		COM_ERROR_IF_FAILED(device->CreateTexture2D(&irradiancedepthDesc, nullptr, irradiancedepthStencilBuffer.GetAddressOf()), "Failed to create texture");
		D3D11_DEPTH_STENCIL_DESC IrradiancedepthStencilDesc = {};
		IrradiancedepthStencilDesc.DepthEnable = TRUE;
		IrradiancedepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		IrradiancedepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

		COM_ERROR_IF_FAILED(device->CreateDepthStencilView(irradiancedepthStencilBuffer.Get(), nullptr, irradiancedepthStencilView.GetAddressOf()), "Failed to create DSV");
		
		D3D11_DEPTH_STENCIL_DESC HDRIdepthStencilDesc = {};
		HDRIdepthStencilDesc.DepthEnable = TRUE;
		HDRIdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		HDRIdepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

		COM_ERROR_IF_FAILED(device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled), "Failed to create Stencil");
	
	
		if (!m_EquiToHDRI_VS.Initialize(device, L"CompiledShaders/EquiToHdri_v.cso", InputElements::posDesc, ARRAYSIZE(InputElements::posDesc)))
		{
			return false;
		}
	
		if (!m_IrradianceConvolution_PS.Initialize(device, L"CompiledShaders/IrradianceConvolution_p.cso"))
		{
			return false;
		}
		D3D11_TEXTURE2D_DESC irradiancetexDesc = {};
		irradiancetexDesc.Width = 32;
		irradiancetexDesc.Height = 32;
		irradiancetexDesc.MipLevels = 1;
		irradiancetexDesc.ArraySize = 6; // 6 faces
		irradiancetexDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDR-capable
		irradiancetexDesc.SampleDesc.Count = 1;
		irradiancetexDesc.Usage = D3D11_USAGE_DEFAULT;
		irradiancetexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		irradiancetexDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		COM_ERROR_IF_FAILED(device->CreateTexture2D(&irradiancetexDesc, nullptr, IrradiancemapTexture.GetAddressOf()), "Failed to create texture");

		D3D11_SHADER_RESOURCE_VIEW_DESC irradiancesrvDesc = {};
		irradiancesrvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		irradiancesrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		irradiancesrvDesc.TextureCube.MipLevels = 1;
		irradiancesrvDesc.TextureCube.MostDetailedMip = 0;

		COM_ERROR_IF_FAILED(device->CreateShaderResourceView(IrradiancemapTexture.Get(), &irradiancesrvDesc, IrradianceMapSRV.GetAddressOf()), "Failed to create SRV");

		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		rtvDesc.Texture2DArray.MipSlice = 0;
		rtvDesc.Texture2DArray.ArraySize = 1;

		for (UINT i = 0; i < 6; ++i)
		{
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			COM_ERROR_IF_FAILED(device->CreateRenderTargetView(IrradiancemapTexture.Get(), &rtvDesc, irradianceRTVs[i].GetAddressOf()), "Failed to create RTV");

		}
	
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
	}
	bool IrradiaceConvolutionPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Initialize(device);
		HDRIViewProj.Initialize(device, deviceContext);

		return true;
	}
	void IrradiaceConvolutionPass::Draw(Graphics* gfx)
	{
		D3D11_VIEWPORT irrviewport = {};
		irrviewport.TopLeftX = 0;
		irrviewport.TopLeftY = 0;
		irrviewport.Width = 32;
		irrviewport.Height = 32;
		irrviewport.MinDepth = 0.0f;
		irrviewport.MaxDepth = 1.0f;

		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		gfx->SetVSConstantBuffers(0, 1, HDRIViewProj.GetAddressOf());
		XMMATRIX views[6] = {
		 XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(1, 0, 0, 0), XMVectorSet(0, 1, 0, 0)), // +X
		XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0)), // -X
		XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 0, -1, 0)), // +Y
		XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0)), // -Y
		XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 1, 0, 0)),  // +Z
		XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 0, -1, 0), XMVectorSet(0, 1, 0, 0)), // -Z
		};

		XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 10.0f);

		for (int i = 0; i < 6; i++)
		{
			gfx->GetDeviceContext()->OMSetRenderTargets(1, irradianceRTVs[i].GetAddressOf(), irradiancedepthStencilView.Get());
			gfx->GetDeviceContext()->ClearRenderTargetView(irradianceRTVs[i].Get(), bgcolor);
			gfx->GetDeviceContext()->RSSetViewports(1, &irrviewport);

			// Set camera matrices (your constant buffer)
			HDRIViewProj.data.View = XMMatrixTranspose(views[i]);
			HDRIViewProj.data.Projection = XMMatrixTranspose(proj);
			HDRIViewProj.ApplyChanges();

			gfx->GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
			gfx->GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

			gfx->GetDeviceContext()->DrawIndexed(36, 0, 0);

		}

		ID3D11RenderTargetView* nullRTVs[1] = { nullptr };
		gfx->GetDeviceContext()->OMSetRenderTargets(1, nullRTVs, nullptr);
		ID3D11SamplerState* nullSampler[1] = { nullptr };
		gfx->GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
		ID3D11Buffer* nullBuffer = nullptr;
		gfx->SetVSConstantBuffers(0, 1, &nullBuffer);
	}

	void IrradiaceConvolutionPass::Draw(Graphics* gfx, ID3D11ShaderResourceView* HDRISRV)
	{


		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		gfx->ClearView(bgcolor);
		gfx->GetDeviceContext()->PSSetSamplers(0, 1,HDRIsamplerState.GetAddressOf());

		gfx->SetInputLayout(this->m_EquiToHDRI_VS.GetInputLayout());
		gfx->SetRasterizerState();
		gfx->SetBlendState();
		gfx->ClearDepthStencil(irradiancedepthStencilView.Get());

		gfx->GetDeviceContext()->OMSetDepthStencilState(HDRIdepthStencilStateDisabled.Get(), 0);
		gfx->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfx->GetDeviceContext()->VSSetShader(m_EquiToHDRI_VS.GetShader(), NULL, 0);
		gfx->GetDeviceContext()->PSSetShader(m_IrradianceConvolution_PS.GetShader(), NULL, 0);

		gfx->GetDeviceContext()->PSSetShaderResources(0, 1, &HDRISRV);

		Draw(gfx);
	}

	void IrradiaceConvolutionPass::ImGuiPass()
	{
		
	}

	std::vector<ID3D11ShaderResourceView*> IrradiaceConvolutionPass::GetSRVRenderTarget()
	{
		return { IrradianceMapSRV.Get() };
	}

}