#include "pch.h"
#include "LightingPass.h"
#include "GBufferPass.h"
#include "InputElements.h"

namespace Engine
{

	bool LightingPass::Initialize(ID3D11Device* device)
	{
		D3D11_SAMPLER_DESC HDRIsampDesc = {};
		HDRIsampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		HDRIsampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		HDRIsampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		HDRIsampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		COM_ERROR_IF_FAILED(device->CreateSamplerState(&HDRIsampDesc, &HDRIsamplerState), "Failed to make hdri sampler");
	
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		sampDesc.BorderColor[0] = 1.0f;
		sampDesc.BorderColor[1] = 1.0f;
		sampDesc.BorderColor[2] = 1.0f;
		sampDesc.BorderColor[3] = 1.0f;


		COM_ERROR_IF_FAILED(device->CreateSamplerState(&sampDesc, &shadowSampler), "Failed to create sampler");
	
		if (!m_DeferredvertexShader.Initialize(device, L"CompiledShaders/DefferedVert_v.cso", InputElements::FullScreenRectlayout, ARRAYSIZE(InputElements::FullScreenRectlayout)))
		{

			return false;
		}

		if (!m_DeferredpixelShader.Initialize(device, L"CompiledShaders/DefferedPixel_p.cso"))
		{
			return false;
		}
		std::vector<FullScreenQuad> vertices = {
			// Positions (x, y, z) and Texture coordinates (u, v)
			{DirectX::XMFLOAT2(-1.0f,  1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)}, // Top-left
			{ DirectX::XMFLOAT2(1.0f,  1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }, // Top-right
			{ DirectX::XMFLOAT2(-1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) }, // Bottom-left
			{ DirectX::XMFLOAT2(1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }, // Bottom-right
		};

		COM_ERROR_IF_FAILED(m_FullScreenVertex.Initialize(device, vertices.data(), vertices.size()), "Failed to create vertex buffer");

		DWORD indices[] = {
			0, 1, 2, // First triangle
			2, 1, 3  // Second triangle
		};
		COM_ERROR_IF_FAILED(m_FullScreenIndex.Initialize(device, indices, 6), "Failed to initialize index buffer.");



		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = Graphics::windowWidth;
		textureDesc.Height = Graphics::windowHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

		// Texture for Position (32-bit floating point RGBA format to store position accurately)
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		device->CreateTexture2D(&textureDesc, nullptr, &FinalTexture);
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
		renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		renderTargetViewDesc.Texture2D.MipSlice = 0;

		if (FAILED(device->CreateRenderTargetView(FinalTexture.Get(), &renderTargetViewDesc, &FinalRTV)))
		{
			std::cout << "Failed to create render target view" << std::endl;
		}
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Texture2D.MostDetailedMip = 0;

		device->CreateShaderResourceView(FinalTexture.Get(), &srvDesc, &FinalSRV);

	}

	bool LightingPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Initialize(device);
		COM_ERROR_IF_FAILED(m_lightparams.Initialize(device, deviceContext), "Failed to initialize constant buffer.");

		COM_ERROR_IF_FAILED(CameraInfoConstantBuffer.Initialize(device, deviceContext) , "Failed to initialize constant buffer.");
	
	
	
	}

	void LightingPass::Draw(Graphics* gfx, GBufferSRV bufSRV, LightingSRVData SRVData )
	{
		D3D11_VIEWPORT viewport;
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(Graphics::windowWidth);
		viewport.Height = static_cast<float>(Graphics::windowHeight);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

		gfx->GetDeviceContext()->OMSetRenderTargets(1, FinalRTV.GetAddressOf(), nullptr);

		gfx->GetDeviceContext()->RSSetViewports(1, &viewport);
		gfx->SetRasterizerState();
		gfx->SetBlendState();
		gfx->GetDeviceContext()->OMSetDepthStencilState(gfx->depthStencilStateDisabled.Get(), 0);
		gfx->SetSamplers();
		gfx->GetDeviceContext()->PSSetSamplers(1, 1, HDRIsamplerState.GetAddressOf());
		gfx->GetDeviceContext()->PSSetSamplers(2, 1, shadowSampler.GetAddressOf());
		gfx->SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());

		gfx->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		gfx->GetDeviceContext()->VSSetShader(m_DeferredvertexShader.GetShader(), NULL, 0);
		gfx->GetDeviceContext()->PSSetShader(m_DeferredpixelShader.GetShader(), NULL, 0);

		std::vector< ID3D11ShaderResourceView*> shaderresources = {
			bufSRV.m_normal,
			bufSRV.m_diffuse,
			bufSRV.m_Specular,
			bufSRV.m_position,
			SRVData.IrradianceSRV,
			SRVData.HDRIFrameBufferSRV,
			SRVData.PrefilteringSRV,
			SRVData.BRDFSRV,
			SRVData.CascadeShadowMapSRV[0],
			SRVData.CascadeShadowMapSRV[1],
			SRVData.CascadeShadowMapSRV[2],
			SRVData.CascadeShadowMapSRV[3],
			bufSRV.m_Depth,
			SRVData.DirShadowMapSRV
		};

		gfx->GetDeviceContext()->PSSetShaderResources(0, shaderresources.size(), shaderresources.data());



		gfx->SetPSConstantBuffers(0, 1, m_lightparams.GetAddressOf());
		CameraInfoConstantBuffer.data.CameraPosition = gfx->camera.GetPositionFloat3();
		CameraInfoConstantBuffer.data.InvProj = XMMatrixTranspose(XMMatrixInverse(nullptr, gfx->camera.GetProjectionMatrix()));
		CameraInfoConstantBuffer.data.InvView = XMMatrixTranspose(XMMatrixInverse(nullptr, gfx->camera.GetViewMatrix()));
		CameraInfoConstantBuffer.data.View = XMMatrixTranspose(gfx->camera.GetViewMatrix());

		CameraInfoConstantBuffer.ApplyChanges();
		gfx->SetPSConstantBuffers(1, 1, CameraInfoConstantBuffer.GetAddressOf());
		gfx->SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());
		gfx->SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		gfx->GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
		gfx->GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

		gfx->GetDeviceContext()->DrawIndexed(6, 0, 0);

		ID3D11ShaderResourceView* nullSRVs[13] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,nullptr, nullptr, nullptr, nullptr };
		gfx->GetDeviceContext()->PSSetShaderResources(0, 12, nullSRVs);
		ID3D11SamplerState* nullSampler[1] = { nullptr };
		gfx->GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
	}

	void LightingPass::Draw(Graphics* gfx)
	{

	}

	void LightingPass::SetLightMatrixBuffers(ShadowPass& shadow)
	{
		std::vector<XMMATRIX> LightMatrcies = shadow.GetCascadeMatrix();

		m_lightparams.data.LightSpaceMatrices0 = LightMatrcies[0];
		m_lightparams.data.LightSpaceMatrices1 = LightMatrcies[1];
		m_lightparams.data.LightSpaceMatrices2 = LightMatrcies[2];
		m_lightparams.data.LightSpaceMatrices3 = LightMatrcies[3];
		m_lightparams.data.cascadePlaneDistances.x = shadow.GetCascadesLevels()[1];
		m_lightparams.data.cascadePlaneDistances.y = shadow.GetCascadesLevels()[2];
		m_lightparams.data.cascadePlaneDistances.z = shadow.GetCascadesLevels()[3];
		m_lightparams.data.cascadePlaneDistances.w = shadow.GetCascadesLevels()[4];
		m_lightparams.data.LSMDirectionalShadow = shadow.GetDirShadowMatrix();
		m_lightparams.data.farplane = shadow.GetCascadesLevels()[5];
		m_lightparams.data.LightDirection = shadow.GetLightDir();
		m_lightparams.data.LightColor = {10,10,10};
		m_lightparams.ApplyChanges();
	}

	void LightingPass::ImGuiPass()
	{
		
	}

	std::vector<ID3D11ShaderResourceView*> LightingPass::GetSRVRenderTarget()
	{
		return {FinalSRV.Get()};
	}

}