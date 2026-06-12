#include "pch.h"

#include "Graphics.h"
#include "Vertex.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Acceleration/BVH/BVH.h"
#define M_PI       3.14159265358979323846

namespace Engine
{



Graphics::~Graphics()
{
	/*for (int i = 0; i < 6; i++)
	{
		delete HDRIFramebufferRTV[i];
	}
	delete[] HDRIFramebufferRTV;*/
}

bool Graphics::Initialize(HWND hwnd, int width, int height)
{
	
	windowWidth = width;
	windowHeight = height;
	if (!InitializeDirectX(hwnd))
		return false;

	if (!InitializeShaders())
		return false;
	if (!InitializeScene())
		return false;

	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();


	return true;
}
void Graphics::PhysicsUpdate()
{
}

void Graphics::Present()
{
	this->swapchain->Present(0, NULL);
}




void Graphics::RenderFrame()
{

}

bool Graphics::InitializeDirectX(HWND hwnd)
{
	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 1.0f, 1000.0f);
	std::vector<AdapterData> adapters = AdapterReader::GetAdapters();

	if (adapters.size() < 1)
	{
		ErrorLogger::Log("No IDXGI Adapters found.");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc.Width = windowWidth;
	scd.BufferDesc.Height = windowHeight;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;

	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 1;
	scd.OutputWindow = hwnd;
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | D3D11_CREATE_DEVICE_DEBUG;
#if defined(_DEBUG)
	scd.Flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(	adapters[0].pAdapter, //IDXGI Adapter
										D3D_DRIVER_TYPE_UNKNOWN,
										NULL, //FOR SOFTWARE DRIVER TYPE
										NULL, //FLAGS FOR RUNTIME LAYERS
										NULL, //FEATURE LEVELS ARRAY
										0, //# OF FEATURE LEVELS IN ARRAY
										D3D11_SDK_VERSION,
										&scd, //Swapchain description
										this->swapchain.GetAddressOf(), //Swapchain Address
										this->device.GetAddressOf(), //Device Address
										NULL, //Supported feature level
										this->deviceContext.GetAddressOf()); //Device Context Address

	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create device and swapchain.");
		return false;
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
	hr = this->swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "GetBuffer Failed.");
		return false;
	}

	hr = device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create render target view.");
		return false;
	}


	//hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	//if (FAILED(hr)) //If error occurred
	//{
	//	ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
	//	return false;
	//}

	




	//hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), &dsvDesc, this->depthStencilView1.GetAddressOf());
	//if (FAILED(hr)) //If error occurred
	//{
	//	ErrorLogger::Log(hr, "Failed to create depth stencil view.");
	//	return false;
	//}



	//this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), depthStencilView.Get());
	
	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;

	hr = this->device->CreateDepthStencilState(&depthstencildesc, this->depthStencilState.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil state.");
		return false;
	}


	//Create the Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = windowWidth;
	viewport.Height = windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	//Set the Viewport
	this->deviceContext->RSSetViewports(1, &viewport);
	
	
	//Create Rasterizer State
	D3D11_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
	hr = this->device->CreateRasterizerState(&rasterizerDesc, this->rasterizerstate.GetAddressOf());
	if (FAILED(hr))
	{
		ErrorLogger::Log(hr, "Failed to create rasterizer state.");
		return false;
	}

	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	device->CreateSamplerState(&sampDesc, &samplerState);

	sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	device->CreateSamplerState(&sampDesc, &DepthsamplerState);





	return true;
}

bool Graphics::InitializeShaders()
{
	return true;
}

bool Graphics::InitializeScene()
{

	
	


	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;              
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; 
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS; 

	depthStencilDesc.StencilEnable = FALSE; // Optional — disables stencil too

	 HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, &depthStencilStateDisabled);
	//HDRI-----------------------------------------
	// D3D11_SAMPLER_DESC samplerDesc = {};
	// samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	// samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;  
	// samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;  
	// samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  
	// samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	// samplerDesc.MinLOD = 0;
	// samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//hr = device->CreateSamplerState(&samplerDesc, &HDRIsamplerState);
	
	


	D3D11_DEPTH_STENCIL_DESC skyboxdepthStencilDesc = {};
	skyboxdepthStencilDesc.DepthEnable = TRUE;
	skyboxdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 
	skyboxdepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&skyboxdepthStencilDesc, &depthStencilSkyboxState);
	
	






	


	 D3D11_BLEND_DESC blendDesc = {};
	 blendDesc.AlphaToCoverageEnable = FALSE;
	 blendDesc.IndependentBlendEnable = FALSE;

	 D3D11_RENDER_TARGET_BLEND_DESC rtBlend = {};
	 rtBlend.BlendEnable = TRUE;
	 rtBlend.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	 rtBlend.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	 rtBlend.BlendOp = D3D11_BLEND_OP_ADD;
	 rtBlend.SrcBlendAlpha = D3D11_BLEND_ONE;
	 rtBlend.DestBlendAlpha = D3D11_BLEND_ZERO;
	 rtBlend.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	 rtBlend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	 blendDesc.RenderTarget[0] = rtBlend;

	 hr = device->CreateBlendState(&blendDesc, transparentBlendState.GetAddressOf());

	/* D3D11_RASTERIZER_DESC rasterizerDesc;
	 ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

	 rasterizerDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	 rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	 
	 hr = this->device->CreateRasterizerState(&rasterizerDesc, this->DebugLineState.GetAddressOf());*/
	 //D3D11_TEXTURE2D_DESC ShadowtexDesc = {};
	 //ShadowtexDesc.Width = depthMapResolution;
	 //ShadowtexDesc.Height = depthMapResolution;
	 //ShadowtexDesc.MipLevels = 1;
	 //ShadowtexDesc.ArraySize = shadowSRVs.size();
	 //ShadowtexDesc.Format = DXGI_FORMAT_R32_TYPELESS; // Or your format
	 //ShadowtexDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	 //ShadowtexDesc.MiscFlags = 0;
	 //ShadowtexDesc.Usage = D3D11_USAGE_DEFAULT;
	 //ShadowtexDesc.SampleDesc.Count = 1;

	 //
	 //device->CreateTexture2D(&ShadowtexDesc, nullptr, &ShadowtextureArray);


	 //for (UINT i = 0; i < shadowSRVs.size(); ++i)
	 //{
		// UINT dstSubresource = D3D11CalcSubresource(0, i, 1);
		// deviceContext->CopySubresourceRegion(
		//	 ShadowtextureArray.Get(), dstSubresource, 0, 0, 0,
		//	 shadowTex[i].Get(), 0, nullptr);
	 //}

	 //D3D11_SHADER_RESOURCE_VIEW_DESC ShadowsrvDesc = {};
	 //ShadowsrvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Or matching your data
	 //ShadowsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	 //ShadowsrvDesc.Texture2DArray.MostDetailedMip = 0;
	 //ShadowsrvDesc.Texture2DArray.MipLevels = 1;
	 //ShadowsrvDesc.Texture2DArray.FirstArraySlice = 0;
	 //ShadowsrvDesc.Texture2DArray.ArraySize = shadowSRVs.size();

	 //
	 //device->CreateShaderResourceView(ShadowtextureArray.Get(), &ShadowsrvDesc, &ShadowtextureArraySRV);


	
	// D3D11_RASTERIZER_DESC rasterDesc = {};
	// rasterDesc.FillMode = D3D11_FILL_SOLID;
	// rasterDesc.CullMode = D3D11_CULL_FRONT; // Cull front faces
	// rasterDesc.DepthClipEnable = true;

	// 
	// device->CreateRasterizerState(&rasterDesc, shadowRasterState.GetAddressOf());
	//
	// D3D11_TEXTURE2D_DESC td = {};
	// td.Width = windowWidth / DownSampleMultiplier;
	// td.Height = windowHeight / DownSampleMultiplier;
	// td.MipLevels = 1;
	// td.ArraySize = 1;
	// td.Format = DXGI_FORMAT_R32_FLOAT;
	// td.SampleDesc.Count = 1;
	// td.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	// 
	// device->CreateTexture2D(&td, nullptr, RaytracedshadowTex.GetAddressOf());

	// D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {};
	// uavd.Format = td.Format;
	// uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	// uavd.Texture2D.MipSlice = 0;
	// 
	// device->CreateUnorderedAccessView(RaytracedshadowTex.Get(), &uavd, shadowUAV.GetAddressOf());
	//
	//D3D11_SHADER_RESOURCE_VIEW_DESC rsrv = {};
	//rsrv.Format = srvDesc.Format;
	//rsrv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;    // a single 2D map
	//rsrv.Texture2D.MipLevels = 1;
	//rsrv.Texture2D.MostDetailedMip = 0;
	//hr = device->CreateShaderResourceView(RaytracedshadowTex.Get(), &srvDesc, RaytracedShadowSRV.GetAddressOf());
	//
	//D3D11_SHADER_RESOURCE_VIEW_DESC DsrvDesc = {};
	//DsrvDesc.Format = DXGI_FORMAT_R32_FLOAT; // Depth only
	//DsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	//DsrvDesc.Texture2D.MostDetailedMip = 0;
	//DsrvDesc.Texture2D.MipLevels = 1;
	//hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &DsrvDesc, DepthBuffer.GetAddressOf());
	//
	
	assert(SUCCEEDED(hr));
	return true;




}

//void Graphics::CalculateCascades()
//{
//	constexpr int SHADOW_MAP_CASCADE_COUNT = 4;
//	m_CascadeLightVP.resize(SHADOW_MAP_CASCADE_COUNT);
//
//	float nearClip = 0.1f;
//	float farClip = 100;
//	float clipRange = farClip - nearClip;
//
//	float minZ = nearClip;
//	float maxZ = nearClip + clipRange;
//	float range = maxZ - minZ;
//	float ratio = maxZ / minZ;
//
//	float cascadeSplits[SHADOW_MAP_CASCADE_COUNT];
//
//	// Use log-uniform blend for split distances
//	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; i++) {
//		float p = (i + 1) / static_cast<float>(SHADOW_MAP_CASCADE_COUNT);
//		float log = minZ * std::pow(ratio, p);
//		float uniform = minZ + range * p;
//		float d = 0.95f * (log - uniform) + uniform;
//		cascadeSplits[i] = (d - nearClip) / clipRange;
//	}
//
//	float lastSplitDist = 0.0f;
//
//	// Camera matrices
//	XMMATRIX camView = camera.GetViewMatrix();
//	XMMATRIX camProj = camera.GetProjectionMatrix();
//	XMMATRIX invCamViewProj = XMMatrixInverse(nullptr, camView * camProj);
//
//	// For each cascade
//	for (uint32_t i = 0; i < SHADOW_MAP_CASCADE_COUNT; ++i) {
//		float splitDist = cascadeSplits[i];
//
//		// Frustum corners in NDC
//		XMFLOAT3 frustumCorners[8] = {
//			{-1,  1, -1}, { 1,  1, -1}, { 1, -1, -1}, {-1, -1, -1},
//			{-1,  1,  1}, { 1,  1,  1}, { 1, -1,  1}, {-1, -1,  1},
//		};
//
//		// Transform corners to world space
//		for (int j = 0; j < 8; ++j) {
//			XMVECTOR v = XMVectorSet(frustumCorners[j].x, frustumCorners[j].y, frustumCorners[j].z, 1.0f);
//			v = XMVector4Transform(v, invCamViewProj);
//			v /= XMVectorGetW(v);
//			XMStoreFloat3(&frustumCorners[j], v);
//		}
//
//		// Slice the frustum
//		for (int j = 0; j < 4; ++j) {
//			XMVECTOR cornerNear = XMLoadFloat3(&frustumCorners[j]);
//			XMVECTOR cornerFar = XMLoadFloat3(&frustumCorners[j + 4]);
//			XMVECTOR dir = cornerFar - cornerNear;
//			XMLoadFloat3(&frustumCorners[j]) = cornerNear + dir * lastSplitDist;
//			XMLoadFloat3(&frustumCorners[j + 4]) = cornerNear + dir * splitDist;
//		}
//
//		lastSplitDist = splitDist;
//
//		// Compute frustum center
//		XMVECTOR center = XMVectorZero();
//		for (int j = 0; j < 8; ++j)
//			center += XMLoadFloat3(&frustumCorners[j]);
//		center /= 8.0f;
//
//		// Light direction
//		XMVECTOR lightDir = -XMVector3Normalize(XMLoadFloat3(&direction));
//		XMVECTOR eye = center - lightDir * 100.0f; // pull back along light direction
//
//		// Light view matrix
//		XMMATRIX lightView = XMMatrixLookAtLH(eye, center, XMVectorSet(0, 0, 1, 0));
//
//		// Project frustum corners into light space to get bounds
//		float minX = FLT_MAX, maxX = -FLT_MAX;
//		float minY = FLT_MAX, maxY = -FLT_MAX;
//		float minZ = FLT_MAX, maxZ = -FLT_MAX;
//
//		for (int j = 0; j < 8; ++j) {
//			XMVECTOR cornerLS = XMVector3TransformCoord(XMLoadFloat3(&frustumCorners[j]), lightView);
//			float x = XMVectorGetX(cornerLS);
//			float y = XMVectorGetY(cornerLS);
//			float z = XMVectorGetZ(cornerLS);
//			minX = std::min(minX, x);
//			maxX = std::max(maxX, x);
//			minY = std::min(minY, y);
//			maxY = std::max(maxY, y);
//			minZ = std::min(minZ, z);
//			maxZ = std::max(maxZ, z);
//		}
//
//		// Optional: stabilize cascade to texel grid
//		float radiusX = (maxX - minX) / 2.0f;
//		float radiusY = (maxY - minY) / 2.0f;
//		float centerX = (minX + maxX) / 2.0f;
//		float centerY = (minY + maxY) / 2.0f;
//		radiusX *= 0.1;
//		radiusY *= 0.1;
//		float texelSizeX = radiusX * 2.0f / depthMapResolution;
//		float texelSizeY = radiusY * 2.0f / depthMapResolution;
//
//		centerX = std::floor(centerX / texelSizeX) * texelSizeX;
//		centerY = std::floor(centerY / texelSizeY) * texelSizeY;
//
//		minX = centerX - radiusX;
//		maxX = centerX + radiusX;
//		minY = centerY - radiusY;
//		maxY = centerY + radiusY;
//
//		// Final ortho projection for this cascade
//		XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
//
//		m_CascadeLightVP[i] = XMMatrixTranspose(lightView * lightProj);
//	}
//}
//std::vector<XMFLOAT3> Graphics::getFrustumCornersWorldSpace(const XMMATRIX& proj, const XMMATRIX& view)
//{
//	return GetFrustumCornersWorldSpace( view * proj);
//}

//XMMATRIX Graphics::getLightSpaceMatrix(const float nearPlane, const float farPlane)
//{
//	// 1. Build split projection matrix for the cascade range
//	const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
//	XMMATRIX splitProj = XMMatrixPerspectiveFovLH(
//		XMConvertToRadians(90.0f),
//		aspect,
//		nearPlane, farPlane);
//
//	XMMATRIX view = camera.GetViewMatrix();
//	XMMATRIX viewProj = view * splitProj;
//
//	std::vector<XMVECTOR> corners = getFrustumCornersWorldSpace(viewProj);
//
//	// 2. Compute frustum center
//	XMVECTOR center = XMVectorZero();
//	for (const auto& pt : corners)
//		center += pt;
//	center /= static_cast<float>(corners.size());
//
//	// 3. Compute light direction
//	float theta = M_PI * Sky.x;
//	float phi = 2 * M_PI * Sky.y;
//	direction = XMFLOAT3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
//
//	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&direction));
//	XMVECTOR eye = center - lightDir * shadowDirstance;
//
//	XMFLOAT3 up_vec = (Sky.x < -0.1f || Sky.x > -0.9f) ? XMFLOAT3(0, 0, 1) : XMFLOAT3(0, 1, 0);
//	XMMATRIX lightView = XMMatrixLookAtLH(eye, center, XMLoadFloat3(&up_vec));
//
//	// 4. Build AABB in light space
//	float minX = FLT_MAX, maxX = -FLT_MAX;
//	float minY = FLT_MAX, maxY = -FLT_MAX;
//	float minZ = FLT_MAX, maxZ = -FLT_MAX;
//
//	for (const auto& corner : corners)
//	{
//		XMVECTOR trf = XMVector3TransformCoord(corner, lightView);
//		XMFLOAT3 pt;
//		XMStoreFloat3(&pt, trf);
//
//		minX = std::min(minX, pt.x); maxX = std::max(maxX, pt.x);
//		minY = std::min(minY, pt.y); maxY = std::max(maxY, pt.y);
//		minZ = std::min(minZ, pt.z); maxZ = std::max(maxZ, pt.z);
//	}
//
//	// 5. Snap to texel grid
//	const float worldUnitsPerTexel = (maxX - minX) / static_cast<float>(depthMapResolution);
//	minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
//	maxX = std::floor(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
//	minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
//	maxY = std::floor(maxY / worldUnitsPerTexel) * worldUnitsPerTexel;
//
//	// 6. Clamp depth
//	minZ = std::max(minZ, -farPlane);
//	maxZ = std::min(maxZ, farPlane);
//
//	// 7. Final ortho projection
//	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
//	return XMMatrixTranspose(lightView * lightProj);
//}
//std::vector<XMMATRIX> Graphics::getLightSpaceMatrices()
//{
//	std::vector<XMMATRIX> ret;
//	for (size_t i = 0; i < NUM_CASCADES; i++)
//	{
//		if (i == 0)
//		{
//			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[0], shadowCascadeLevels[i]));
//		}
//		else if (i < NUM_CASCADES-1)
//		{
//			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
//		}
//		else
//		{
//			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[5]));
//		}
//	}
//	return ret;
//}
std::vector<XMVECTOR> Graphics::getFrustumCornersWorldSpace(const XMMATRIX& projview)
{
	XMMATRIX inv = XMMatrixInverse(nullptr, projview);
	std::vector<XMVECTOR> frustumCorners;
	frustumCorners.reserve(8);

	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				// Clip space point
				XMVECTOR pt = XMVectorSet(
					2.0f * x - 1.0f,
					2.0f * y - 1.0f,
					2.0f * z - 1.0f,
					1.0f
				);

				// Transform to world space
				XMVECTOR world = XMVector4Transform(pt, inv);
				world = XMVectorDivide(world, XMVectorSplatW(world)); // Divide by w

				frustumCorners.push_back(world);
			}
		}
	}

	return frustumCorners;
}
void Graphics::ClearDepthStencil(ID3D11DepthStencilView* stencil)
{
	this->deviceContext->ClearDepthStencilView(stencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

}
void Graphics::SetInputLayout(ID3D11InputLayout* layout)
{
	this->deviceContext->IASetInputLayout(layout);

}
void Graphics::SetTopology(D3D11_PRIMITIVE_TOPOLOGY top)
{
	this->deviceContext->IASetPrimitiveTopology(top);
}
void Graphics::SetRasterizerState()
{
	this->deviceContext->RSSetState(this->rasterizerstate.Get());
}
void Graphics::SetDepthStencilState()
{
	this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
}
void Graphics::SetBlendState()
{
	this->deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
}
void Graphics::SetSamplers()
{
	this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
}
void Graphics::SetPSShader(ID3D11PixelShader* shader)
{
	this->deviceContext->PSSetShader(shader, NULL, 0);
}
void Graphics::SetVSShader(ID3D11VertexShader* shader)
{
	this->deviceContext->VSSetShader(shader, NULL, 0);
}

void Graphics::SetPSConstantBuffers(UINT startSlot, UINT NumOfBuffers, ID3D11Buffer*const* ppBuffer)
{
	this->deviceContext->PSSetConstantBuffers(startSlot, NumOfBuffers, ppBuffer);
}
void Graphics::SetVSConstantBuffers(UINT startSlot, UINT NumOfBuffers, ID3D11Buffer* const* ppBuffer)
{
	this->deviceContext->VSSetConstantBuffers(startSlot, NumOfBuffers, ppBuffer);
}
void Graphics::ClearView(float color[4])
{
	this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), color);
}


}