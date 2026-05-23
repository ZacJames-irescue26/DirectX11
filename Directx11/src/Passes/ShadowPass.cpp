#include "pch.h"
#include "ShadowPass.h"
#include "InputElements.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "ImGui\imgui.h"

namespace Engine
{

	bool ShadowPass::Initialize(ID3D11Device* device)
	{
		//CASCADED SHADOW MAPS----------------------------------------
		shadowTex.resize(NUM_CASCADES);
		shadowDSVs.resize(NUM_CASCADES);
		shadowSRVs.resize(NUM_CASCADES);
		for (int i = 0; i < NUM_CASCADES; ++i)
		{
			D3D11_TEXTURE2D_DESC desc = {};
			desc.Width = depthMapResolution;
			desc.Height = depthMapResolution;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.SampleDesc.Count = 1;
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

			COM_ERROR_IF_FAILED(device->CreateTexture2D(&desc, nullptr, shadowTex[i].GetAddressOf()), "Failed to create texture");
			// Depth view
			D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
			dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
			dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
			COM_ERROR_IF_FAILED(device->CreateDepthStencilView(shadowTex[i].Get(), &dsvDesc, &shadowDSVs[i]), "Failed to create DSV");
			// SRV for shader access
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MipLevels = 1;
			COM_ERROR_IF_FAILED(device->CreateShaderResourceView(shadowTex[i].Get(), &srvDesc, &shadowSRVs[i]), "Failed to create SRV");
		}

		D3D11_DEPTH_STENCIL_DESC ShadowdepthStencilDesc = {};
		ShadowdepthStencilDesc.DepthEnable = TRUE;                                 // Enable depth testing
		ShadowdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;        // Allow depth writes
		ShadowdepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;            // Standard comparison

		ShadowdepthStencilDesc.StencilEnable = FALSE;                              // We don’t use stencil here

		COM_ERROR_IF_FAILED(device->CreateDepthStencilState(&ShadowdepthStencilDesc, shadowDepthStencilState.GetAddressOf()), "Failed to create DSS");
	
		if (!m_ShadowDepth_VS.Initialize(device, L"CompiledShaders/ShadowMapDepth_v.cso", InputElements::ModelPos, ARRAYSIZE(InputElements::ModelPos)))
		{
			return false;
		}
		if (!m_ShadowDepth_GS.Initialize(device, L"CompiledShaders/ShadowMapDepth_g.cso"))
		{
			return false;
		}
	
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



		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = depthMapResolution;
		desc.Height = depthMapResolution;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

		COM_ERROR_IF_FAILED(device->CreateTexture2D(&desc, nullptr, DirectionalshadowTex.GetAddressOf()), "Failed to create texture");
		// Depth view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		COM_ERROR_IF_FAILED(device->CreateDepthStencilView(DirectionalshadowTex.Get(), &dsvDesc, &DirectionalshadowDSVs), "Failed to create DSV");
		// SRV for shader access
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		COM_ERROR_IF_FAILED(device->CreateShaderResourceView(DirectionalshadowTex.Get(), &srvDesc, &DirectionalshadowSRVs), "Failed to create SRV");



		return true;
	}

	bool ShadowPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
	{
		Initialize(device);
		COM_ERROR_IF_FAILED(m_ObjectModel.Initialize(device, deviceContext), "Failed to initialize constant buffer.");
		
		COM_ERROR_IF_FAILED(m_LightSpace.Initialize(device, deviceContext), "Failed to initialize constant buffer.");
		
		return true;
	}
	void ShadowPass::CalcCascadeOrthoProjs(Graphics* gfx)
	{
		// 1) Inverse of camera view
		XMMATRIX invCamView = XMMatrixInverse(nullptr, gfx->camera.GetViewMatrix());

		// 2) Build a "light view" matrix looking along m_LightDir from the origin
		//    (for directional light we treat eye at 0,0,0)
		XMVECTOR eyePos = XMVectorSet(gfx->camera.GetPositionFloat3().x, gfx->camera.GetPositionFloat3().y, gfx->camera.GetPositionFloat3().z, 0.0);
		XMVECTOR lightFwd = XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z, 1.0));
		XMVECTOR lightUp = XMVectorSet(0, 1, 0, 0);
		// if lightDir is (almost) colinear with up, pick another up

		if (fabsf(XMVectorGetX(XMVector3Dot(lightFwd, lightUp))) > 0.99f)
			lightUp = XMVectorSet(1, 0, 0, 0);
		XMMATRIX lightView = XMMatrixLookAtLH(eyePos, lightFwd, lightUp);

		// 3) Precompute FOV tangents
		//    NOTE: m_CameraProj was built with XMMatrixPerspectiveFovLH(fov, aspect, near, far)
		//    So half-vertical FOV = fov/2, and aspect = width/height.
		float fov = 90; // recover fov from proj
		float aspect = gfx->windowWidth / gfx->windowHeight;
		float tanHFOV = tanf(fov * 0.5f);
		float tanVFOV = tanHFOV / aspect;
		m_CascadeLightVP.resize(NUM_CASCADES);
		// 4) For each cascade slice

		for (int i = 0; i < NUM_CASCADES; ++i)
		{
			float zn = shadowCascadeLevels[i];
			float zf = shadowCascadeLevels[i + 1];

			// view?space frustum extents at those depths
			float xn = zn * tanHFOV, xf = zf * tanHFOV;
			float yn = zn * tanVFOV, yf = zf * tanVFOV;

			// the 8 corners in view space
			XMVECTOR frustumVS[8] = {
				// near plane
				XMVectorSet(xn,  yn, zn, 1),
				XMVectorSet(-xn,  yn, zn, 1),
				XMVectorSet(xn, -yn, zn, 1),
				XMVectorSet(-xn, -yn, zn, 1),
				// far plane
				XMVectorSet(xf,  yf, zf, 1),
				XMVectorSet(-xf,  yf, zf, 1),
				XMVectorSet(xf, -yf, zf, 1),
				XMVectorSet(-xf, -yf, zf, 1),
			};

			// transform into light?space and find AABB
			float minX = +FLT_MAX, maxX = -FLT_MAX;
			float minY = +FLT_MAX, maxY = -FLT_MAX;
			float minZ = +FLT_MAX, maxZ = -FLT_MAX;

			for (int c = 0; c < 8; ++c)
			{
				// view ? world
				XMVECTOR cornerWS = XMVector3TransformCoord(frustumVS[c], invCamView);
				// world ? light
				XMVECTOR cornerLS = XMVector3TransformCoord(cornerWS, lightView);

				minX = min(minX, XMVectorGetX(cornerLS));
				maxX = max(maxX, XMVectorGetX(cornerLS));
				minY = min(minY, XMVectorGetY(cornerLS));
				maxY = max(maxY, XMVectorGetY(cornerLS));
				minZ = min(minZ, XMVectorGetZ(cornerLS));
				maxZ = max(maxZ, XMVectorGetZ(cornerLS));
			}

			// 5) (Optional) Snap to texel grid to reduce shimmering
			float worldUnitsPerTexel = (maxX - minX) / depthMapResolution;
			minX = floorf(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
			maxX = floorf(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
			worldUnitsPerTexel = (maxY - minY) / depthMapResolution;
			minY = floorf(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
			maxY = floorf(maxY / worldUnitsPerTexel) * worldUnitsPerTexel;

			// 6) Build the cascade?specific ortho proj
			XMMATRIX cascadeProj = XMMatrixOrthographicOffCenterLH(
				minX, maxX,
				minY, maxY,
				minZ, maxZ);

			// 7) Store view×proj for use when rendering this cascade
			m_CascadeLightVP[i] = XMMatrixTranspose(lightView * cascadeProj);
		}
	}
	void ShadowPass::Draw(Graphics* gfx, Scene& scene)
	{
		{

			CalcCascadeOrthoProjs(gfx);
			const std::vector<XMMATRIX> lightMatrices = m_CascadeLightVP;
			for (UINT i = 0; i < NUM_CASCADES; i++)
			{
				D3D11_VIEWPORT shadowViewport = {};
				shadowViewport.TopLeftX = 0;
				shadowViewport.TopLeftY = 0;
				shadowViewport.Width = depthMapResolution;
				shadowViewport.Height = depthMapResolution;
				shadowViewport.MinDepth = 0.0f;
				shadowViewport.MaxDepth = 1.0f;
				// 1. Set viewport (matching your shadow map resolution)

				// 2. Set depth-only render target for this cascade
				gfx->GetDeviceContext()->OMSetRenderTargets(0, nullptr, shadowDSVs[i].Get());

				gfx->SetRasterizerState();
				gfx->SetBlendState();
				gfx->GetDeviceContext()->RSSetViewports(1, &shadowViewport);
				// 3. Clear depth buffer
				gfx->GetDeviceContext()->ClearDepthStencilView(shadowDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
				gfx->GetDeviceContext()->OMSetDepthStencilState(shadowDepthStencilState.Get(), 0);

				// 4. Set shaders
				gfx->GetDeviceContext()->VSSetShader(m_ShadowDepth_VS.GetShader(), nullptr, 0);
				gfx->GetDeviceContext()->GSSetShader(nullptr, nullptr, 0); // Optional: geometry shader to set gl_Layer
				gfx->SetInputLayout(this->m_ShadowDepth_VS.GetInputLayout());
				gfx->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);           // No pixel shader for depth-only
				//gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_LightSpace.GetAddressOf());
				gfx->GetDeviceContext()->VSSetConstantBuffers(1, 1, m_LightSpace.GetAddressOf());
				// 5. Set view/projection matrix for this cascade

				m_LightSpace.data.LightSpace = lightMatrices[i];

				m_LightSpace.ApplyChanges();
				// 6. Draw scene
				gfx->GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ObjectModel.GetAddressOf());
		
				scene.DrawWithoutCBuffer(&m_ObjectModel);
			}

			gfx->GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);

		}
		{

			XMMATRIX ShadowProj = XMMatrixPerspectiveFovLH(
				XMConvertToRadians(60.0f), 1.77, 0.1, farplane);
			//XMMATRIX ShadowOrtho = XMMatrixOrthographicOffCenterLH(-30.0f, 30.00, -30.0f, 30.0f, 0.01, farplane);
			float theta = M_PI * Sky.x;
			float phi = 2 * M_PI * Sky.y;
			direction = XMFLOAT3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
			XMFLOAT3 up_vec = XMFLOAT3(0.0, 1.0, 0.0);
			if (Sky.x < -0.1 || Sky.x > -0.9) up_vec = XMFLOAT3(0.0, 0.0, 1.0);

			XMVECTOR target = XMLoadFloat3(&TargetVec);

			XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&direction));

			XMVECTOR eye = XMVectorMultiplyAdd(lightDir, XMVectorReplicate(shadowDirstance), { 0.0,0.0,0.0 });

			XMMATRIX lightView = XMMatrixLookAtLH(eye, { 0.0,0.0,0.0 }, XMLoadFloat3(&up_vec));

			float minX = 1, maxX = 10, minY = 1, maxY = 10;



			const float SM_SIZE = 2048.0f;


			float w = maxX - minX;
			float h = maxY - minY;
			float texelW = w / SM_SIZE;
			float texelH = h / SM_SIZE;


			minX = std::floor(minX / texelW) * texelW;
			minY = std::floor(minY / texelH) * texelH;

			maxX = minX + texelW * SM_SIZE;
			maxY = minY + texelH * SM_SIZE;


			XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
				minX, maxX,
				minY, maxY,
				0.01, farplane);


			XMMATRIX lightViewProj = lightView * lightProj;
			lightMatrices = XMMatrixTranspose(lightViewProj);


			D3D11_VIEWPORT shadowViewport = {};
			shadowViewport.TopLeftX = 0;
			shadowViewport.TopLeftY = 0;
			shadowViewport.Width = depthMapResolution;
			shadowViewport.Height = depthMapResolution;
			shadowViewport.MinDepth = 0.0f;
			shadowViewport.MaxDepth = 1.0f;
			// 1. Set viewport (matching your shadow map resolution)
			// 2. Set depth-only render target for this cascade
			gfx->GetDeviceContext()->OMSetRenderTargets(0, nullptr, DirectionalshadowDSVs.Get());
			gfx->SetRasterizerState();
			gfx->SetBlendState();
			gfx->GetDeviceContext()->RSSetViewports(1, &shadowViewport);
			// 3. Clear depth buffer
			gfx->GetDeviceContext()->ClearDepthStencilView(DirectionalshadowDSVs.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
			gfx->GetDeviceContext()->OMSetDepthStencilState(shadowDepthStencilState.Get(), 0);
			// 4. Set shaders
			gfx->GetDeviceContext()->VSSetShader(m_ShadowDepth_VS.GetShader(), nullptr, 0);
			gfx->SetInputLayout(this->m_ShadowDepth_VS.GetInputLayout());
			gfx->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);           // No pixel shader for depth-only
			//gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_LightSpace.GetAddressOf());
			gfx->GetDeviceContext()->VSSetConstantBuffers(1, 1, m_LightSpace.GetAddressOf());
			// 5. Set view/projection matrix for this cascade
			m_LightSpace.data.LightSpace = lightMatrices;
			m_LightSpace.ApplyChanges();
			// 6. Draw scene
			gfx->GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ObjectModel.GetAddressOf());

			scene.DrawWithoutCBuffer(&m_ObjectModel);

		}




	}

	void ShadowPass::Draw(Graphics* gfx)
	{

	}

	void ShadowPass::ImGuiPass()
	{
		ImGui::Begin("Shadow props");
		if (ImGui::TreeNode("Dir Light"))
		{
			//ImGui::DragFloat3("Light direction", &m_lightparams.data.LightDirection.x, 0.1f, -1000.0f, 1000.0f);
			ImGui::DragFloat3("direction", &direction.x, 0.1f, -1000.0f, 1000.0f);
			ImGui::DragFloat2("SkyDir", &Sky.x, 0.001, -1, 1);
			ImGui::DragFloat("shadow distance ", &shadowDirstance, 1, 1, 10000);
			ImGui::DragFloat("farplane ", &farplane, 1, 1, 10000);
			ImGui::Image((ImTextureID)DirectionalshadowSRVs.Get(), { 200,200 });
			ImGui::Image((ImTextureID)shadowSRVs[0].Get(), { 200,200 });
			ImGui::Image((ImTextureID)shadowSRVs[1].Get(), { 200,200 });
			ImGui::Image((ImTextureID)shadowSRVs[2].Get(), { 200,200 });
			ImGui::Image((ImTextureID)shadowSRVs[3].Get(), { 200,200 });
			ImGui::TreePop();
		}
		//ImGui::Image((ImTextureID)gfx.positionSRV.Get(), { 200,200 });
		//ImGui::Image((ImTextureID)gfx.DirectionalshadowSRVs.Get(), { 200,200 });

		ImGui::End();
	}

	std::vector<ID3D11ShaderResourceView*> ShadowPass::GetSRVRenderTarget()
	{
		return {nullptr};
	}

}