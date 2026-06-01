#include "pch.h"
#include "DrawHDRIPass.h"
#include "InputElements.h"

namespace Engine
{

	bool HDRIPass::Initialize(ID3D11Device* device)
	{


		if (!m_EquiToHDRI_VS.Initialize(device, (LPCWSTR)Project::GetEditorShaderPath("EquiToHdri_v.cso").wstring().c_str(), InputElements::posDesc, ARRAYSIZE(InputElements::posDesc)))
		{
			return false;
		}
		if (!m_EquiToHdri_PS.Initialize(device, (LPCWSTR)Project::GetEditorShaderPath("EquiToHdri_p.cso").wstring().c_str()))
		{
			return false;
		}

		HDRIFilepath = Engine::Project::ResolveAssetPath("HDRI/kloppenheim_06_puresky_4k.hdr").string();

		D3D11_SAMPLER_DESC HDRIsampDesc = {};
		HDRIsampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		HDRIsampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		HDRIsampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		HDRIsampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		COM_ERROR_IF_FAILED(device->CreateSamplerState(&HDRIsampDesc, &HDRIsamplerState), "Failed to make hdri sampler");

		D3D11_DEPTH_STENCIL_DESC HDRIdepthStencilDesc = {};
		HDRIdepthStencilDesc.DepthEnable = TRUE;
		HDRIdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		HDRIdepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;

		COM_ERROR_IF_FAILED(device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled), "Failed to create Stencil");

		int width, height, nrComponents;
		float* data = stbi_loadf(HDRIFilepath.c_str(), &width, &height, &nrComponents, 0);
		if (data == nullptr)
		{
			ErrorLogger::Log("Failed to load hdri image");
		}
		D3D11_TEXTURE2D_DESC texDesc = {};
		texDesc.Width = width;
		texDesc.Height = height;
		texDesc.MipLevels = 1;
		texDesc.ArraySize = 1;
		texDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT; // HDR format
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = data;
		initData.SysMemPitch = width * sizeof(float) * 3;

		COM_ERROR_IF_FAILED(device->CreateTexture2D(&texDesc, &initData, HDRITexture.GetAddressOf()), "Failed to create texture");

		stbi_image_free(data);


		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;

		COM_ERROR_IF_FAILED(device->CreateShaderResourceView(HDRITexture.Get(), &srvDesc, HDRISRV.GetAddressOf()), "Failed to create SRV");


		texDesc = {};
		texDesc.Width = 512; // e.g., 512
		texDesc.Height = 512;
		texDesc.MipLevels = 0; // 0 = generate full mip chain
		texDesc.ArraySize = 6; // 6 faces
		texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDR-capable
		texDesc.SampleDesc.Count = 1;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; //


		COM_ERROR_IF_FAILED(device->CreateTexture2D(&texDesc, nullptr, HDRIFramebufferTexture.GetAddressOf()), "Failed to create texture");

		for (int i = 0; i < 6; ++i)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
			rtvDesc.Format = texDesc.Format;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			srvDesc.TextureCube.MipLevels = -1;
			srvDesc.TextureCube.MostDetailedMip = 0;
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			rtvDesc.Texture2DArray.ArraySize = 1;

			COM_ERROR_IF_FAILED(device->CreateRenderTargetView(HDRIFramebufferTexture.Get(), &rtvDesc, &HDRIFramebufferRTV[i]), "Failed to create RTV");
		}
		D3D11_SHADER_RESOURCE_VIEW_DESC HDRIsrvDesc = {};
		HDRIsrvDesc.Format = texDesc.Format;
		HDRIsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		HDRIsrvDesc.TextureCube.MipLevels = -1;
		HDRIsrvDesc.TextureCube.MostDetailedMip = 0;

		COM_ERROR_IF_FAILED(device->CreateShaderResourceView(HDRIFramebufferTexture.Get(), &HDRIsrvDesc, HDRIFramebufferSRV.GetAddressOf()),"Failed to create SRV");

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

		
		COM_ERROR_IF_FAILED(m_HdriVertex.Initialize(device, cubevertices.data(), cubevertices.size()) , "Failed to initialize Vertex buffer.");
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

	bool HDRIPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Initialize(device);
		HDRIViewProj.Initialize(device, deviceContext);

		return true;
	}

	void HDRIPass::Draw(Graphics* gfx)
	{
		D3D11_VIEWPORT hdviewport = {};
		hdviewport.TopLeftX = 0;
		hdviewport.TopLeftY = 0;
		hdviewport.Width = 512;
		hdviewport.Height = 512;
		hdviewport.MinDepth = 0.0f;
		hdviewport.MaxDepth = 1.0f;
		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
		gfx->deviceContext->PSSetSamplers(0, 1, HDRIsamplerState.GetAddressOf());

		gfx->GetDeviceContext()->VSSetShader(m_EquiToHDRI_VS.GetShader(), NULL, 0);
		gfx->GetDeviceContext()->PSSetShader(m_EquiToHdri_PS.GetShader(), NULL, 0);
		gfx->SetInputLayout(this->m_EquiToHDRI_VS.GetInputLayout());
		gfx->SetRasterizerState();
		gfx->SetBlendState();
		gfx->GetDeviceContext()->OMSetDepthStencilState(HDRIdepthStencilStateDisabled.Get(), 0);
		gfx->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		gfx->GetDeviceContext()->PSSetShaderResources(0, 1, HDRISRV.GetAddressOf());

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
			gfx->GetDeviceContext()->RSSetViewports(1, &hdviewport);
			gfx->GetDeviceContext()->OMSetRenderTargets(1, HDRIFramebufferRTV[i].GetAddressOf(), nullptr);
			gfx->GetDeviceContext()->ClearRenderTargetView(HDRIFramebufferRTV[i].Get(), bgcolor);


			// Set camera matrices (your constant buffer)
			HDRIViewProj.data.View = XMMatrixTranspose(views[i]);
			HDRIViewProj.data.Projection = XMMatrixTranspose(proj);
			HDRIViewProj.ApplyChanges();
			
			gfx->GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
			gfx->GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

			gfx->GetDeviceContext()->DrawIndexed(36, 0, 0);

		}
		gfx->GetDeviceContext()->GenerateMips(HDRIFramebufferSRV.Get());
		ID3D11RenderTargetView* nullRTVs[1] = { nullptr };
		gfx->GetDeviceContext()->OMSetRenderTargets(1, nullRTVs, nullptr);
		ID3D11SamplerState* nullSampler[1] = { nullptr };
		gfx->GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
		ID3D11Buffer* nullBuffer = nullptr;
		gfx->SetVSConstantBuffers(0, 1, &nullBuffer);
	}

	void HDRIPass::ImGuiPass()
	{
		
	}

	std::vector<ID3D11ShaderResourceView*> HDRIPass::GetSRVRenderTarget()
	{
		return {HDRIFramebufferSRV.Get()};
	}

}