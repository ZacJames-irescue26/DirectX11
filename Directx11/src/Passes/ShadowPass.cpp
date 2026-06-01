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
	
		if (!m_ShadowDepth_VS.Initialize(device, Project::GetEditorShaderPath("ShadowMapDepth_v.cso").wstring().c_str(), InputElements::ModelPos, ARRAYSIZE(InputElements::ModelPos)))
		{
			return false;
		}
		if (!m_ShadowDepth_GS.Initialize(device, Project::GetEditorShaderPath("ShadowMapDepth_g.cso").wstring()))
		{
			return false;
		}
	
		D3D11_SAMPLER_DESC sampDesc = {};
		sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;

		// Reversed depth:
		sampDesc.ComparisonFunc = D3D11_COMPARISON_GREATER_EQUAL;

		// Outside the shadow map should be treated as lit.
		// For reversed depth, far/clear depth is 0, but for comparison sampling
		// a border of 0 can fail GREATER_EQUAL checks.
		// Use 0 if you want outside to be shadowed, 1 if you want outside lit.
		// Usually for shadow maps, outside should be lit:
		sampDesc.BorderColor[0] = 0.0f;
		sampDesc.BorderColor[1] = 0.0f;
		sampDesc.BorderColor[2] = 0.0f;
		sampDesc.BorderColor[3] = 0.0f;

		COM_ERROR_IF_FAILED(
			device->CreateSamplerState(&sampDesc, &shadowSampler),
			"Failed to create sampler"
		);



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

		D3D11_DEPTH_STENCIL_DESC depthdesc = {};
		depthdesc.DepthEnable = TRUE;
		depthdesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthdesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
		depthdesc.StencilEnable = FALSE;

		device->CreateDepthStencilState(&depthdesc, shadowDepthStencilState.GetAddressOf());

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
		m_CascadeLightVP.resize(NUM_CASCADES);

		XMMATRIX camView = gfx->camera.GetViewMatrix();
		XMMATRIX invCamView = XMMatrixInverse(nullptr, camView);

		float fov = XMConvertToRadians(90); // use real FOV
		float aspect = static_cast<float>(Graphics::windowWidth) / static_cast<float>(Graphics::windowHeight);

		float tanY = tanf(fov * 0.5f);
		float tanX = tanY * aspect;

		XMVECTOR lightDir = XMVector3Normalize(
			XMVectorSet(direction.x, direction.y, direction.z, 0.0f)
		);

		XMVECTOR lightUp = XMVectorSet(0, 1, 0, 0);

		if (fabsf(XMVectorGetX(XMVector3Dot(lightDir, lightUp))) > 0.99f)
			lightUp = XMVectorSet(1, 0, 0, 0);

		for (int i = 0; i < NUM_CASCADES; i++)
		{
			float zn = shadowCascadeLevels[i];
			float zf = shadowCascadeLevels[i + 1];

			float xn = zn * tanX;
			float xf = zf * tanX;
			float yn = zn * tanY;
			float yf = zf * tanY;

			XMVECTOR cornersWS[8] =
			{
				XMVector3TransformCoord(XMVectorSet(-xn,  yn, zn, 1), invCamView),
				XMVector3TransformCoord(XMVectorSet(xn,  yn, zn, 1), invCamView),
				XMVector3TransformCoord(XMVectorSet(-xn, -yn, zn, 1), invCamView),
				XMVector3TransformCoord(XMVectorSet(xn, -yn, zn, 1), invCamView),

				XMVector3TransformCoord(XMVectorSet(-xf,  yf, zf, 1), invCamView),
				XMVector3TransformCoord(XMVectorSet(xf,  yf, zf, 1), invCamView),
				XMVector3TransformCoord(XMVectorSet(-xf, -yf, zf, 1), invCamView),
				XMVector3TransformCoord(XMVectorSet(xf, -yf, zf, 1), invCamView),
			};

			XMVECTOR center = XMVectorZero();

			for (int c = 0; c < 8; c++)
				center += cornersWS[c];

			center /= 8.0f;

			float radius = 0.0f;

			for (int c = 0; c < 8; c++)
			{
				float d = XMVectorGetX(XMVector3Length(cornersWS[c] - center));
				radius = std::max(radius, d);
			}

			radius = std::ceil(radius * 16.0f) / 16.0f;

			float lightDistance = radius * 4.0f;

			XMVECTOR eye = center - lightDir * lightDistance;
			XMVECTOR target = center;

			XMMATRIX lightView = XMMatrixLookAtLH(eye, target, lightUp);

			XMVECTOR centerLS = XMVector3TransformCoord(center, lightView);

			float centerX = XMVectorGetX(centerLS);
			float centerY = XMVectorGetY(centerLS);
			float centerZ = XMVectorGetZ(centerLS);

			float extent = radius;

			float worldUnitsPerTexel = (extent * 2.0f) / static_cast<float>(depthMapResolution);

			centerX = std::floor(centerX / worldUnitsPerTexel) * worldUnitsPerTexel;
			centerY = std::floor(centerY / worldUnitsPerTexel) * worldUnitsPerTexel;

			float minX = centerX - extent;
			float maxX = centerX + extent;
			float minY = centerY - extent;
			float maxY = centerY + extent;

			// Use a generous Z range around the cascade center.
			// This allows objects outside the camera frustum to cast shadows into it.
			float nearZ = centerZ - lightDistance;
			float farZ = centerZ + lightDistance;

			// Reversed-depth orthographic projection:
			// far first, near second.
			XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
				minX, maxX,
				minY, maxY,
				farZ,
				nearZ
			);

			XMMATRIX lightVP = lightView * lightProj;

			// HLSL uses mul(float4(pos, 1), matrix), default float4x4.
			m_CascadeLightVP[i] = XMMatrixTranspose(lightVP);
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
				gfx->GetDeviceContext()->ClearDepthStencilView(shadowDSVs[i].Get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
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

			XMFLOAT3 lightDirection = direction;

			XMVECTOR lightDir = XMVector3Normalize(
				XMVectorSet(lightDirection.x, lightDirection.y, lightDirection.z, 0.0f)
			);

			// Distance backward along the light direction.
			float lightDistance = 200.0f;
			XMFLOAT3 camPos = {0.0,0.0,0.0};
			
			XMVECTOR center = XMLoadFloat3(&camPos);
			center = XMVectorSetY(center, 0.0);
			XMVECTOR eye = center - lightDir * shadowDirstance;
			XMVECTOR target = center;

			XMVECTOR up = XMVectorSet(0, 1, 0, 0);

			if (fabsf(XMVectorGetX(XMVector3Dot(lightDir, up))) > 0.99f)
			{
				up = XMVectorSet(1, 0, 0, 0);
			}

			XMMATRIX lightView = XMMatrixLookAtLH(eye, target, up);

			// Big enough to cover Sponza while debugging.
			float orthoSize = 150.0f;

			float shadowMapSize = static_cast<float>(depthMapResolution);
			float worldUnitsPerTexel = (orthoSize * 2.0f) / shadowMapSize;

			XMVECTOR centerLS = XMVector3TransformCoord(center, lightView);

			float centerX = XMVectorGetX(centerLS);
			float centerY = XMVectorGetY(centerLS);

			centerX = floorf(centerX / worldUnitsPerTexel) * worldUnitsPerTexel;
			centerY = floorf(centerY / worldUnitsPerTexel) * worldUnitsPerTexel;

			float minX = centerX - orthoSize;
			float maxX = centerX + orthoSize;
			float minY = centerY - orthoSize;
			float maxY = centerY + orthoSize;

			XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(
				minX, maxX,
				minY, maxY,
				farplane,
				nearplane
			);

			XMMATRIX lightViewProj = lightView * lightProj;


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
			gfx->GetDeviceContext()->ClearDepthStencilView(DirectionalshadowDSVs.Get(), D3D11_CLEAR_DEPTH, 0.0f, 0);
			gfx->GetDeviceContext()->OMSetDepthStencilState(shadowDepthStencilState.Get(), 0);
			// 4. Set shaders
			gfx->GetDeviceContext()->VSSetShader(m_ShadowDepth_VS.GetShader(), nullptr, 0);
			gfx->SetInputLayout(this->m_ShadowDepth_VS.GetInputLayout());
			gfx->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);           // No pixel shader for depth-only
			//gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_LightSpace.GetAddressOf());
			gfx->GetDeviceContext()->VSSetConstantBuffers(1, 1, m_LightSpace.GetAddressOf());
			// 5. Set view/projection matrix for this cascade
			m_LightSpace.data.LightSpace = XMMatrixTranspose(lightViewProj);
			lightMatrix = m_LightSpace.data.LightSpace;
			m_LightSpace.ApplyChanges();
			// 6. Draw scene
			gfx->GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ObjectModel.GetAddressOf());

			scene.DrawWithoutCBuffer(&m_ObjectModel);

			ID3D11RenderTargetView* nullview[] = { nullptr };
			gfx->GetDeviceContext()->OMSetRenderTargets(1, nullview, nullptr);


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
			
			ImGui::DragFloat3("direction", &direction.x, 0.1f);
			ImGui::DragFloat2("SkyDir", &Sky.x, 0.001, -1, 1);
			ImGui::DragFloat("shadow distance ", &shadowDirstance, 1, 1, 10000);
			ImGui::DragFloat("farplane ", &farplane, 1, 1, 10000);
			
			
			ImGui::DragFloat2("Min", &Shadowmin.x, 0.1);
			ImGui::DragFloat2("Max", &Shadowmax.x, 0.1);
			
			
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