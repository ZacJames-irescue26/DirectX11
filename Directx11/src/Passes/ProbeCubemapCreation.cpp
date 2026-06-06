#include "pch.h"
#include "ProbeCubemapCreation.h"
#include "..\Camera.h"
#include "InputElements.h"
#include "illumination\Surfel.h"
#include "Acceleration\Octree\Octree.h"
#include "illumination\SurfelGenerator.h"
#include "src/Graphics/Color.h"
namespace Engine
{

	bool ProbeCubemapCreationPass::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, uint32_t faceSize, const std::vector<Probe>& probes)
	{
		if (!device || faceSize <= 0)
			return false;

		FaceSize = faceSize;
		probeCount = probes.size();


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





		auto CreateCubeRT = [&](DXGI_FORMAT format,
			Microsoft::WRL::ComPtr<ID3D11Texture2D>& texture,
			Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv[6],
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv,
			Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& Computesrv) -> bool
			{
				D3D11_TEXTURE2D_DESC texDesc = {};
				texDesc.Width = faceSize;
				texDesc.Height = faceSize;
				texDesc.MipLevels = 1;
				texDesc.ArraySize = 6;
				texDesc.Format = format;
				texDesc.SampleDesc.Count = 1;
				texDesc.SampleDesc.Quality = 0;
				texDesc.Usage = D3D11_USAGE_DEFAULT;
				texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
				texDesc.CPUAccessFlags = 0;
				texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

				HRESULT hr = device->CreateTexture2D(&texDesc, nullptr, texture.GetAddressOf());
				if (FAILED(hr))
					return false;

				for (int face = 0; face < 6; ++face)
				{
					D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
					rtvDesc.Format = format;
					rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
					rtvDesc.Texture2DArray.MipSlice = 0;
					rtvDesc.Texture2DArray.FirstArraySlice = face;
					rtvDesc.Texture2DArray.ArraySize = 1;

					hr = device->CreateRenderTargetView(
						texture.Get(),
						&rtvDesc,
						rtv[face].GetAddressOf()
					);

					if (FAILED(hr))
						return false;
				}

				D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
				srvDesc.Format = format;
				srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				srvDesc.TextureCube.MostDetailedMip = 0;
				srvDesc.TextureCube.MipLevels = 1;

				hr = device->CreateShaderResourceView(
					texture.Get(),
					&srvDesc,
					srv.GetAddressOf()
				);
				D3D11_SHADER_RESOURCE_VIEW_DESC arraySrvDesc = {};
				arraySrvDesc.Format = format;
				arraySrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
				arraySrvDesc.Texture2DArray.MostDetailedMip = 0;
				arraySrvDesc.Texture2DArray.MipLevels = 1;
				arraySrvDesc.Texture2DArray.FirstArraySlice = 0;
				arraySrvDesc.Texture2DArray.ArraySize = 6;

				hr = device->CreateShaderResourceView(
					texture.Get(),
					&arraySrvDesc,
					Computesrv.GetAddressOf()
				);
				if (FAILED(hr))
					return false;

				return true;
			};

		// Normal cubemap
		if (!CreateCubeRT(
			DXGI_FORMAT_R16G16B16A16_FLOAT,
			NormalTexture,
			NormalRTV,
			NormalSRV,
			ComputeNormalSRV))
		{
			return false;
		}

		// Albedo cubemap
		if (!CreateCubeRT(
			DXGI_FORMAT_R8G8B8A8_UNORM,
			AlbedoTexture,
			AlbedoRTV,
			AlbedoSRV,
			ComputeAlbedoSRV))
		{
			return false;
		}

		// Depth cubemap
		{
			D3D11_TEXTURE2D_DESC depthDesc = {};
			depthDesc.Width = faceSize;
			depthDesc.Height = faceSize;
			depthDesc.MipLevels = 1;
			depthDesc.ArraySize = 6;
			depthDesc.Format = DXGI_FORMAT_R32_TYPELESS;
			depthDesc.SampleDesc.Count = 1;
			depthDesc.SampleDesc.Quality = 0;
			depthDesc.Usage = D3D11_USAGE_DEFAULT;
			depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
			depthDesc.CPUAccessFlags = 0;
			depthDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			HRESULT hr = device->CreateTexture2D(
				&depthDesc,
				nullptr,
				DepthTexture.GetAddressOf()
			);

			if (FAILED(hr))
				return false;

			for (int face = 0; face < 6; ++face)
			{
				D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
				dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
				dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
				dsvDesc.Texture2DArray.MipSlice = 0;
				dsvDesc.Texture2DArray.FirstArraySlice = face;
				dsvDesc.Texture2DArray.ArraySize = 1;

				hr = device->CreateDepthStencilView(
					DepthTexture.Get(),
					&dsvDesc,
					DepthDSV[face].GetAddressOf()
				);

				if (FAILED(hr))
					return false;
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
			depthSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			depthSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			depthSrvDesc.TextureCube.MostDetailedMip = 0;
			depthSrvDesc.TextureCube.MipLevels = 1;

			hr = device->CreateShaderResourceView(
				DepthTexture.Get(),
				&depthSrvDesc,
				DepthSRV.GetAddressOf()
			);
			
			D3D11_SHADER_RESOURCE_VIEW_DESC arraySrvDesc = {};
			arraySrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
			arraySrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
			arraySrvDesc.Texture2DArray.MostDetailedMip = 0;
			arraySrvDesc.Texture2DArray.MipLevels = 1;
			arraySrvDesc.Texture2DArray.FirstArraySlice = 0;
			arraySrvDesc.Texture2DArray.ArraySize = 6;

			hr = device->CreateShaderResourceView(
				DepthTexture.Get(),
				&arraySrvDesc,
				ComputeDepthSRV.GetAddressOf()
			);

			COM_ERROR_IF_FAILED(hr, "Hr failed");
		}
		D3D11_DEPTH_STENCIL_DESC depthstencildesc = {};
		
		depthstencildesc.DepthEnable = true;
		depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthstencildesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

		auto hr = device->CreateDepthStencilState(&depthstencildesc, DSS.GetAddressOf());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to create depth stencil state.");
			return false;
		}

		if (!m_ProbeGBuffer.Initialize(device, Project::GetEditorShaderPath("ProbeGBuffer_v.cso").wstring().c_str(), InputElements::layout, ARRAYSIZE(InputElements::layout)))
		{

			return false;
		}
		if (!m_ProbeGBufferpixelShader.Initialize(device, Project::GetEditorShaderPath("ProbeGBuffer_p.cso").wstring().c_str()))
		{
			return false;
		}
		/*if (!m_ProbeCS.Initialize(device, Project::GetEditorShaderPath("CreateSurfelForProbe_c.cso").wstring().c_str()))
		{
			return false;
		}*/
		if (!m_SurfelLightingCS.Initialize(device, Project::GetEditorShaderPath("LightingSurfels_c.cso").wstring().c_str()))
		{
			return false;
		}
		if (!m_AccumilateProbesCS.Initialize(device, Project::GetEditorShaderPath("AccumilateProbe_c.cso").wstring().c_str()))
		{
			return false;
		}
		m_ConstantBufferSurfelCreation.Initialize(device, deviceContext);


		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(Probe) * probes.size();
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(Probe);

		COM_ERROR_IF_FAILED(device->CreateBuffer(&desc, nullptr, ProbeBuffer.GetAddressOf()), "Failed to create buffer");

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = probes.size();

		COM_ERROR_IF_FAILED(device->CreateUnorderedAccessView(ProbeBuffer.Get(), &uavDesc, ProbeUAV.GetAddressOf()), "Failed to create UAV");

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = probes.size();

		COM_ERROR_IF_FAILED(device->CreateShaderResourceView(
			ProbeBuffer.Get(),
			&srvDesc,
			ProbeSRV.GetAddressOf()
		), "Failed to create SRV");

		std::vector<Probe> gpuProbes;
		gpuProbes.resize(probes.size());

		for (size_t i = 0; i < probes.size(); ++i)
		{
			gpuProbes[i] = {};
			gpuProbes[i].Position = probes[i].Position;
			gpuProbes[i].CaptureRadius = probes[i].CaptureRadius;
		}

		deviceContext->UpdateSubresource(
			ProbeBuffer.Get(),
			0,
			nullptr,
			gpuProbes.data(),
			0,
			0
		);

		uint32_t accumElementCount = static_cast<uint32_t>(probes.size()) * 6;

		desc = {};
		desc.ByteWidth = sizeof(uint32_t) * 4 * accumElementCount;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(uint32_t) * 4;

		COM_ERROR_IF_FAILED(
			device->CreateBuffer(
				&desc,
				nullptr,
				ProbeAccumBuffer.GetAddressOf()
			),
			"Failed to create ProbeAccum buffer"
		);

		uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = accumElementCount;

		COM_ERROR_IF_FAILED(
			device->CreateUnorderedAccessView(
				ProbeAccumBuffer.Get(),
				&uavDesc,
				ProbeAccumUAV.GetAddressOf()
			),
			"Failed to create ProbeAccum UAV"
		);

		srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = accumElementCount;

		COM_ERROR_IF_FAILED(
			device->CreateShaderResourceView(
				ProbeAccumBuffer.Get(),
				&srvDesc,
				ProbeAccumSRV.GetAddressOf()
			),
			"Failed to create ProbeAccum SRV"
		);

		std::vector<XMFLOAT4> probePositions;
		probePositions.reserve(probes.size());

		for (const Probe& p : probes)
		{
			probePositions.push_back({
				p.Position.x,
				p.Position.y,
				p.Position.z,
				p.CaptureRadius
				});
		}

		desc = {};
		desc.ByteWidth = sizeof(XMFLOAT4) * probePositions.size();
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(XMFLOAT4);

		D3D11_SUBRESOURCE_DATA data = {};
		data.pSysMem = probePositions.data();

		device->CreateBuffer(&desc, &data, ProbePositionBuffer.GetAddressOf());

		srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = static_cast<UINT>(probePositions.size());

		device->CreateShaderResourceView(
			ProbePositionBuffer.Get(),
			&srvDesc,
			ProbePositionSRV.GetAddressOf()
		);


		if (!CreateStagingTexture(device, AlbedoTexture.Get(), AlbedoStaging))
			return false;

		if (!CreateStagingTexture(device, NormalTexture.Get(), NormalStaging))
			return false;

		if (!CreateStagingTexture(device, DepthTexture.Get(), DepthStaging))
			return false;

		COM_ERROR_IF_FAILED(m_ConstantBufferSurfelLighting.Initialize(device, deviceContext), "Failed to make constant buffer");
		COM_ERROR_IF_FAILED(m_ConstantBufferProbeAccum.Initialize(device, deviceContext), "Failed to make constant buffer");



		return true;
	}

	void ProbeCubemapCreationPass::BeginFace(
		ID3D11DeviceContext* context,
		int face)
	{
		ID3D11RenderTargetView* rtvs[] =
		{
			AlbedoRTV[face].Get(),
			NormalRTV[face].Get(),
		};

		context->OMSetRenderTargets(
			2,
			rtvs,
			DepthDSV[face].Get()
		);

		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<float>(FaceSize);
		viewport.Height = static_cast<float>(FaceSize);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		context->RSSetViewports(1, &viewport);

		const float clearAlbedo[4] = { 0, 0, 0, 0 };
		const float clearNormal[4] = { 0, 0, 0, 0 };
		context->OMSetDepthStencilState(DSS.Get(), 0);
		context->ClearRenderTargetView(AlbedoRTV[face].Get(), clearAlbedo);
		context->ClearRenderTargetView(NormalRTV[face].Get(), clearNormal);
		context->ClearDepthStencilView(
			DepthDSV[face].Get(),
			D3D11_CLEAR_DEPTH,
			0.0f,
			0
		);
	}
	bool ProbeCubemapCreationPass::Initialize(ID3D11Device* device)
	{
		return false;
	}

	Camera  ProbeCubemapCreationPass::MakeCubemapFaceCamera(
		const XMFLOAT3& probePosition,
		int face,
		float nearPlane,
		float farPlane)
	{
		

		static const XMFLOAT3 Forward[6] =
		{
			XMFLOAT3(1,  0,  0), // +X
			XMFLOAT3(-1,  0,  0), // -X
			XMFLOAT3(0,  1,  0), // +Y
			XMFLOAT3(0, -1,  0), // -Y
			XMFLOAT3(0,  0,  1), // +Z
			XMFLOAT3(0,  0, -1)  // -Z
		};

		static const XMFLOAT3 Up[6] =
		{
			XMFLOAT3(0, -1,  0), // +X
			XMFLOAT3(0, -1,  0), // -X
			XMFLOAT3(0,  0,  1), // +Y
			XMFLOAT3(0,  0, -1), // -Y
			XMFLOAT3(0, -1,  0), // +Z
			XMFLOAT3(0, -1,  0)  // -Z
		};

		XMVECTOR eye = XMLoadFloat3(&probePosition);

		XMVECTOR forward = XMLoadFloat3(&Forward[face]);
		XMVECTOR up = XMLoadFloat3(&Up[face]);

		XMVECTOR target = XMVectorAdd(eye, forward);

		XMMATRIX view = XMMatrixLookAtLH(
			eye,
			target,
			up
		);

		Camera cam;
		cam.SetPosition(probePosition.x,probePosition.y, probePosition.z);
		cam.SetViewMartix(view);
		cam.SetProjectionValues(XM_PIDIV2, 1.0, farPlane, nearPlane);
		return cam;
	}
	
	bool ProbeCubemapCreationPass::CreateStagingTexture(
		ID3D11Device* device,
		ID3D11Texture2D* source,
		Microsoft::WRL::ComPtr<ID3D11Texture2D>& staging)
	{
		if (!device || !source)
			return false;

		D3D11_TEXTURE2D_DESC desc = {};
		source->GetDesc(&desc);

		desc.Usage = D3D11_USAGE_STAGING;
		desc.BindFlags = 0;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		desc.MiscFlags = 0;

		HRESULT hr = device->CreateTexture2D(
			&desc,
			nullptr,
			staging.GetAddressOf()
		);

		return SUCCEEDED(hr);
	}
	
	
	XMFLOAT3 ReconstructWorldPositionFromDepth(
		int x,
		int y,
		float depth,
		int faceSize,
		const XMMATRIX& invViewProj)
	{
		

		float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(faceSize);
		float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(faceSize);

		float ndcX = u * 2.0f - 1.0f;
		float ndcY = 1.0f - v * 2.0f;

		XMVECTOR clipPos = XMVectorSet(ndcX, ndcY, depth, 1.0f);

		XMVECTOR worldH = XMVector4Transform(clipPos, invViewProj);

		float w = XMVectorGetW(worldH);
		if (fabsf(w) < 1e-6f)
			return XMFLOAT3(0, 0, 0);

		worldH = XMVectorScale(worldH, 1.0f / w);

		XMFLOAT3 result;
		XMStoreFloat3(&result, worldH);
		return result;
	}

	XMFLOAT3 DecodeNormal(float r, float g, float b)
	{
		

		XMVECTOR n = XMVectorSet(
			r * 2.0f - 1.0f,
			g * 2.0f - 1.0f,
			b * 2.0f - 1.0f,
			0.0f);

		n = XMVector3Normalize(n);

		XMFLOAT3 result;
		XMStoreFloat3(&result, n);
		return result;
	}
	
	float SRGBToLinear(float c)
	{
		if (c <= 0.04045f)
			return c / 12.92f;

		return powf((c + 0.055f) / 1.055f, 2.4f);
	}
	
	XMFLOAT3 DecodeAlbedoRGBA8(
		uint8_t r,
		uint8_t g,
		uint8_t b,
		bool sourceIsSRGB)
	{
		float rf = r / 255.0f;
		float gf = g / 255.0f;
		float bf = b / 255.0f;

		if (sourceIsSRGB)
		{
			rf = SRGBToLinear(rf);
			gf = SRGBToLinear(gf);
			bf = SRGBToLinear(bf);
		}

		return XMFLOAT3(rf, gf, bf);
	}

	float EstimateSurfelArea(
		const XMFLOAT3& probePosition,
		const XMFLOAT3& worldPosition,
		const XMFLOAT3& normal,
		int faceSize,
		int pixelStep)
	{
		

		XMVECTOR probe = XMLoadFloat3(&probePosition);
		XMVECTOR pos = XMLoadFloat3(&worldPosition);
		XMVECTOR n = XMVector3Normalize(XMLoadFloat3(&normal));

		XMVECTOR toProbe = XMVectorSubtract(probe, pos);

		float dist = XMVectorGetX(XMVector3Length(toProbe));
		if (dist <= 1e-4f)
			return 0.0f;

		XMVECTOR dirToProbe = XMVectorScale(toProbe, 1.0f / dist);

		float cosTheta = fabsf(XMVectorGetX(XMVector3Dot(n, dirToProbe)));
		cosTheta = std::max(cosTheta, 0.2f);

		float pixelSolidAngle =
			4.0f * XM_PI *
			static_cast<float>(pixelStep * pixelStep) /
			static_cast<float>(6 * faceSize * faceSize);

		float area = dist * dist * pixelSolidAngle / cosTheta;

		return std::clamp(area, 0.0001f, 0.05f);
	}
	void ProbeCubemapCreationPass::Draw(Graphics* gfx)
	{
		
	}
	void ProbeCubemapCreationPass::GenerateAndMergeSurfelsFromCubemapCPU(
		ID3D11DeviceContext* context,
		const Probe& probe,
		uint32_t probeIndex,
		const XMMATRIX invViewProjCPU[6],
		Engine::SurfelBuilder& builder,
		int pixelStep,
		bool reversedZ,
		bool albedoIsSRGB)
	{


		if (!context || pixelStep <= 0)
			return;

		// The render targets must already be unbound before this.
		context->CopyResource(AlbedoStaging.Get(), AlbedoTexture.Get());
		context->CopyResource(NormalStaging.Get(), NormalTexture.Get());
		context->CopyResource(DepthStaging.Get(), DepthTexture.Get());

		for (int face = 0; face < 6; ++face)
		{
			UINT subresource = D3D11CalcSubresource(
				0,      // mip
				face,   // array slice
				1       // mip levels
			);

			D3D11_MAPPED_SUBRESOURCE mappedAlbedo = {};
			D3D11_MAPPED_SUBRESOURCE mappedNormal = {};
			D3D11_MAPPED_SUBRESOURCE mappedDepth = {};

			HRESULT hrA = context->Map(
				AlbedoStaging.Get(),
				subresource,
				D3D11_MAP_READ,
				0,
				&mappedAlbedo
			);

			HRESULT hrN = context->Map(
				NormalStaging.Get(),
				subresource,
				D3D11_MAP_READ,
				0,
				&mappedNormal
			);

			HRESULT hrD = context->Map(
				DepthStaging.Get(),
				subresource,
				D3D11_MAP_READ,
				0,
				&mappedDepth
			);

			if (FAILED(hrA) || FAILED(hrN) || FAILED(hrD))
			{
				if (SUCCEEDED(hrA))
					context->Unmap(AlbedoStaging.Get(), subresource);

				if (SUCCEEDED(hrN))
					context->Unmap(NormalStaging.Get(), subresource);

				if (SUCCEEDED(hrD))
					context->Unmap(DepthStaging.Get(), subresource);

				continue;
			}

			for (int y = 0; y < static_cast<int>(FaceSize); y += pixelStep)
			{
				const uint8_t* albedoRow =
					reinterpret_cast<const uint8_t*>(mappedAlbedo.pData) +
					y * mappedAlbedo.RowPitch;

				const uint8_t* normalRow =
					reinterpret_cast<const uint8_t*>(mappedNormal.pData) +
					y * mappedNormal.RowPitch;

				const uint8_t* depthRow =
					reinterpret_cast<const uint8_t*>(mappedDepth.pData) +
					y * mappedDepth.RowPitch;

				for (int x = 0; x < static_cast<int>(FaceSize); x += pixelStep)
				{
					const RGBA8* albedoPixel =
						reinterpret_cast<const RGBA8*>(albedoRow) + x;

					const RGBA16F* normalPixel =
						reinterpret_cast<const RGBA16F*>(normalRow) + x;

					const float* depthPixel =
						reinterpret_cast<const float*>(depthRow) + x;

					float depth = *depthPixel;

					bool invalidDepth = reversedZ
						? depth <= 0.00001f
						: depth >= 0.99999f;

					if (invalidDepth)
						continue;

					float normalAlpha =
						DirectX::PackedVector::XMConvertHalfToFloat(normalPixel->a);

					// Your normal RT should clear alpha to 0 and write alpha = 1 for geometry.
					if (normalAlpha < 0.5f)
						continue;

					float nr = DirectX::PackedVector::XMConvertHalfToFloat(normalPixel->r);
					float ng = DirectX::PackedVector::XMConvertHalfToFloat(normalPixel->g);
					float nb = DirectX::PackedVector::XMConvertHalfToFloat(normalPixel->b);

					XMFLOAT3 normal = DecodeNormal(nr, ng, nb);

					XMFLOAT3 albedo = DecodeAlbedoRGBA8(
						albedoPixel->r,
						albedoPixel->g,
						albedoPixel->b,
						albedoIsSRGB
					);

					XMFLOAT3 position =
						ReconstructWorldPositionFromDepth(
							x,
							y,
							depth,
							static_cast<int>(FaceSize),
							invViewProjCPU[face]
						);

					float area =
						EstimateSurfelArea(
							probe.Position,
							position,
							normal,
							static_cast<int>(FaceSize),
							pixelStep
						);

					if (area <= 0.0f)
						continue;

					Surfel candidate = {};
					candidate.position = position;
					candidate.normal = normal;
					candidate.albedo = XMFLOAT4(albedo.x, albedo.y, albedo.z, 1.0f);
					candidate.radius = area;

					// This is where the octree/builder does the sharing/merging.
					builder.AddOrMerge(candidate);
				}
			}

			context->Unmap(AlbedoStaging.Get(), subresource);
			context->Unmap(NormalStaging.Get(), subresource);
			context->Unmap(DepthStaging.Get(), subresource);
		}
	

	}


	bool ProbeCubemapCreationPass::UploadMergedSurfelsToGPU(
		ID3D11Device* device,
		const std::vector<Surfel>& mergedSurfels)
	{
		if (!device)
			return false;

		GlobalSurfelBuffer.Reset();
		GlobalSurfelSRV.Reset();
		GlobalSurfelCount = 0;

		if (mergedSurfels.empty())
			return false;

		std::vector<GPUSurfel> gpuSurfels;
		gpuSurfels.reserve(mergedSurfels.size());

		for (const Surfel& s : mergedSurfels)
		{
			GPUSurfel gpu = {};

			gpu.Position = s.position;
			gpu.Area = s.radius;

			gpu.Normal = s.normal;
			gpu.Padding0 = 0.0f;

			gpu.Albedo = XMFLOAT3(s.albedo.x, s.albedo.y, s.albedo.z);
			gpu.Padding1 = 0.0f;

			gpuSurfels.push_back(gpu);
		}

		GlobalSurfelCount =
			static_cast<uint32_t>(gpuSurfels.size());

		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth =
			static_cast<UINT>(sizeof(GPUSurfel) * gpuSurfels.size());

		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		bufferDesc.StructureByteStride = sizeof(GPUSurfel);

		D3D11_SUBRESOURCE_DATA initData = {};
		initData.pSysMem = gpuSurfels.data();

		HRESULT hr = device->CreateBuffer(
			&bufferDesc,
			&initData,
			GlobalSurfelBuffer.GetAddressOf()
		);

		if (FAILED(hr))
		{
			GlobalSurfelCount = 0;
			return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = GlobalSurfelCount;

		hr = device->CreateShaderResourceView(
			GlobalSurfelBuffer.Get(),
			&srvDesc,
			GlobalSurfelSRV.GetAddressOf()
		);

		if (FAILED(hr))
		{
			GlobalSurfelBuffer.Reset();
			GlobalSurfelCount = 0;
			return false;
		}

		return true;
	}



	void ProbeCubemapCreationPass::Draw(Graphics* gfx, const std::vector<Probe>& probes, Scene& scene)
	{
		ID3D11DeviceContext* context = gfx->GetDeviceContext();

		
		AABB sceneBounds = AABB(XMFLOAT3(-50,-5,-100), XMFLOAT3(50,20,100));

		Engine::Octree surfelOctree(sceneBounds, 6);
		Engine::SurfelBuilder builder(surfelOctree);

		const int pixelStep = 4;
		const bool reversedZ = true;
		const bool albedoIsSRGB = false;

		for (uint32_t probeIndex = 0; probeIndex < probes.size(); ++probeIndex)
		{
			const Probe& probe = probes[probeIndex];

			XMMATRIX invViewProjCPU[6];

			gfx->SetInputLayout(m_ProbeGBuffer.GetInputLayout());
			gfx->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			gfx->SetRasterizerState();
			gfx->SetBlendState();
			gfx->SetSamplers();

			context->VSSetShader(m_ProbeGBuffer.GetShader(), nullptr, 0);
			context->PSSetShader(m_ProbeGBufferpixelShader.GetShader(), nullptr, 0);

			for (int face = 0; face < 6; ++face)
			{
				BeginFace(context, face);

				Camera faceCamera = MakeCubemapFaceCamera(
					probe.Position,
					face,
					0.1f,
					probe.CaptureRadius
				);

				XMMATRIX viewProj =
					faceCamera.GetViewMatrix() *
					faceCamera.GetProjectionMatrix();

				invViewProjCPU[face] =
					XMMatrixInverse(nullptr, viewProj);

				scene.DrawStaticScene(viewProj);
			}

			context->OMSetRenderTargets(0, nullptr, nullptr);

			GenerateAndMergeSurfelsFromCubemapCPU(
				context,
				probe,
				probeIndex,
				invViewProjCPU,
				builder,
				pixelStep,
				reversedZ,
				albedoIsSRGB
			);
		}

		const std::vector<Surfel>& mergedSurfels = builder.GetSurfels();

		ErrorLogger::Log("Merged surfel count: " + std::to_string(mergedSurfels.size()));

		UploadMergedSurfelsToGPU(gfx->GetDevice(), mergedSurfels);
	}


	void ProbeCubemapCreationPass::LightSurfels(
		Graphics* gfx,
		ID3D11ShaderResourceView* ShadowSRV,
		XMMATRIX Lightspacematrix,
		XMFLOAT3 LightDir,
		XMFLOAT3 LightColor)
	{
		ID3D11DeviceContext* context = gfx->GetDeviceContext();

		// Clear accumulation BEFORE lighting.
		UINT clearValue[4] = { 0, 0, 0, 0 };
		context->ClearUnorderedAccessViewUint(
			ProbeAccumUAV.Get(),
			clearValue
		);

		m_ConstantBufferSurfelLighting.data.LightColor = LightColor;
		m_ConstantBufferSurfelLighting.data.LightDirection = LightDir;
		m_ConstantBufferSurfelLighting.data.LightIntensity = 5.0f;
		m_ConstantBufferSurfelLighting.data.ProbeCount = probeCount;
		m_ConstantBufferSurfelLighting.data.RadianceScale = RadianceScale;
		m_ConstantBufferSurfelLighting.data.SurfelCount = GlobalSurfelCount;
		m_ConstantBufferSurfelLighting.data.ProbeInfluenceRadius = 40.0f; // start here
		m_ConstantBufferSurfelLighting.data.LightSpaceMatrix = Lightspacematrix;
		m_ConstantBufferSurfelLighting.ApplyChanges();

		ID3D11Buffer* cbs[] =
		{
			m_ConstantBufferSurfelLighting.Get()
		};

		context->CSSetConstantBuffers(0, 1, cbs);

		ID3D11ShaderResourceView* srvs[] =
		{
			GlobalSurfelSRV.Get(),   // t0
			ProbePositionSRV.Get(),   // t1
			ShadowSRV
		};

		context->CSSetShaderResources(0, 3, srvs);
		context->CSSetSamplers(0,1, shadowSampler.GetAddressOf());
		ID3D11UnorderedAccessView* uavs[] =
		{
			ProbeAccumUAV.Get()
		};

		UINT initialCounts[] = { UINT(-1) };

		context->CSSetUnorderedAccessViews(
			0,
			1,
			uavs,
			initialCounts
		);

		context->CSSetShader(
			m_SurfelLightingCS.GetShader(),
			nullptr,
			0
		);

		uint32_t groupCount = (GlobalSurfelCount + 127) / 128;

		if (groupCount > 0)
		{
			context->Dispatch(groupCount, 1, 1);
		}

		// IMPORTANT: unbind ProbeAccum as UAV before normalize reads it as SRV.
		ID3D11UnorderedAccessView* nullUAVs[1] = {};
		UINT nullCounts[1] = { UINT(-1) };
		context->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullCounts);

		ID3D11ShaderResourceView* nullSRVs[1] = {};
		context->CSSetShaderResources(0, 1, nullSRVs);

		context->CSSetShader(nullptr, nullptr, 0);
	}
	void ProbeCubemapCreationPass::NormalizeAndAccumilate(Graphics* gfx)
	{
		ID3D11DeviceContext* context = gfx->GetDeviceContext();

		m_ConstantBufferProbeAccum.data.InvRadianceScale =
			1.0f / RadianceScale;

		m_ConstantBufferProbeAccum.data.ProbeCount = probeCount;
		m_ConstantBufferProbeAccum.ApplyChanges();

		ID3D11Buffer* cbs[] =
		{
			m_ConstantBufferProbeAccum.Get()
		};

		context->CSSetConstantBuffers(0, 1, cbs);

		// Read accumulated uint lighting.
		ID3D11ShaderResourceView* srvs[] =
		{
			ProbeAccumSRV.Get()
		};

		context->CSSetShaderResources(0, 1, srvs);

		// Write final float probe lighting.
		ID3D11UnorderedAccessView* uavs[] =
		{
			ProbeUAV.Get()
		};

		UINT initialCounts[] = { UINT(-1) };

		context->CSSetUnorderedAccessViews(0,1,uavs,initialCounts);

		context->CSSetShader(m_AccumilateProbesCS.GetShader(), nullptr, 0);

		context->Dispatch((probeCount + 63) / 64, 1, 1);

		// Unbind.
		ID3D11ShaderResourceView* nullSRVs[1] = {};
		context->CSSetShaderResources(0, 1, nullSRVs);

		ID3D11UnorderedAccessView* nullUAVs[1] = {};
		UINT nullCounts[1] = { UINT(-1) };
		context->CSSetUnorderedAccessViews(0, 1, nullUAVs, nullCounts);

		context->CSSetShader(nullptr, nullptr, 0);
	}
	void ProbeCubemapCreationPass::ImGuiPass()
	{
		
	}

	std::vector<ID3D11ShaderResourceView*> ProbeCubemapCreationPass::GetSRVRenderTarget()
	{
		return {nullptr};
	}

}