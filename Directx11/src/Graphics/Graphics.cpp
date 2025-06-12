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
Microsoft::WRL::ComPtr<ID3D11Device> Graphics::device = nullptr;


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
	physicsController.initialise();

	if (!InitializeShaders())
		return false;
	if (!InitializeScene())
		return false;

	//Setup ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(this->device.Get(), this->deviceContext.Get());
	ImGui::StyleColorsDark();


	return true;
}
void Graphics::PhysicsUpdate()
{
	physicsController.Update();
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
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 1.0f, 100.0f);
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

	hr = this->device->CreateRenderTargetView(backBuffer.Get(), NULL, this->renderTargetView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create render target view.");
		return false;
	}
	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = windowWidth;
	depthStencilDesc.Height = windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
		return false;
	}
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;   // Stencil-capable view
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	
	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), &dsvDesc, this->depthStencilView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil view.");
		return false;
	}
	//this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), depthStencilView.Get());
	
	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

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


	D3D11_SAMPLER_DESC HDRIsampDesc = {};
	 HDRIsampDesc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	 HDRIsampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	 HDRIsampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	 HDRIsampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	if (FAILED(device->CreateSamplerState(&HDRIsampDesc, &HDRIsamplerState)))
	{
		std::cout << "Failed to make hdri sampler" << std::endl;
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

	hr = device->CreateSamplerState(&presamplerDesc, &PrefilteredsamplerState);

	return true;
}

bool Graphics::InitializeShaders()
{
	return true;
}

bool Graphics::InitializeScene()
{

	//GBuffer--------------------------------------------------------------
	// Common settings for G-buffer textures
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = windowWidth;
	textureDesc.Height = windowHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	
	// Texture for Position (32-bit floating point RGBA format to store position accurately)
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	device->CreateTexture2D(&textureDesc, nullptr, &positionTexture);

	// Texture for Normal (32-bit RGBA for storing normal data accurately)
	textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	device->CreateTexture2D(&textureDesc, nullptr, &NormalTexture);

	// Texture for Normal (32-bit RGBA for storing normal data accurately)
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateTexture2D(&textureDesc, nullptr, &SpecularTexture);

	// Texture for Albedo (8-bit RGBA as it's only color data)
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateTexture2D(&textureDesc, nullptr, &DiffuseTexture);

	// Now create Render Target Views (RTVs) for each texture
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc = {};
	renderTargetViewDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;
	
	if (FAILED(device->CreateRenderTargetView(positionTexture.Get(), &renderTargetViewDesc, &positionRTV)))
	{
		std::cout << "Failed to create render target view" << std::endl;
	}
	renderTargetViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	device->CreateRenderTargetView(NormalTexture.Get(), &renderTargetViewDesc, &NormalRTV);
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateRenderTargetView(DiffuseTexture.Get(), &renderTargetViewDesc, &DiffuseRTV);
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateRenderTargetView(SpecularTexture.Get(), &renderTargetViewDesc, &SpecularRTV);

	// Position SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	
	device->CreateShaderResourceView(positionTexture.Get(), &srvDesc, &positionSRV);

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	device->CreateShaderResourceView(NormalTexture.Get(), &srvDesc, &NormalSRV);


	// Specular SRV
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateShaderResourceView(SpecularTexture.Get(), &srvDesc, &SpecularSRV);

	// Diffuse SRV
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	device->CreateShaderResourceView(DiffuseTexture.Get(), &srvDesc, &DiffuseSRV);
	


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
	
	
	D3D11_DEPTH_STENCIL_DESC HDRIdepthStencilDesc = {};
	HDRIdepthStencilDesc.DepthEnable = TRUE;
	HDRIdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	HDRIdepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
	
	hr = device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled);
	
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = 512; // e.g., 512
	texDesc.Height = 512;
	texDesc.MipLevels = 0; // 0 = generate full mip chain
	texDesc.ArraySize = 6; // 6 faces
	texDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // HDR-capable
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET; //

	
	hr = device->CreateTexture2D(&texDesc, nullptr, HDRIFramebufferTexture.GetAddressOf());

	for (int i = 0; i < 6; ++i)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = texDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
		srvDesc.TextureCube.MipLevels = -1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		rtvDesc.Texture2DArray.ArraySize = 1;

		device->CreateRenderTargetView(HDRIFramebufferTexture.Get(), &rtvDesc, &HDRIFramebufferRTV[i]);
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC HDRIsrvDesc = {};
	HDRIsrvDesc.Format = texDesc.Format;
	HDRIsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	HDRIsrvDesc.TextureCube.MipLevels = -1;
	HDRIsrvDesc.TextureCube.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(HDRIFramebufferTexture.Get(), &HDRIsrvDesc, HDRIFramebufferSRV.GetAddressOf());
	
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
	
	hr = device->CreateTexture2D(&irradiancetexDesc, nullptr, IrradiancemapTexture.GetAddressOf());
	
	D3D11_SHADER_RESOURCE_VIEW_DESC irradiancesrvDesc = {};
	irradiancesrvDesc.Format = texDesc.Format;
	irradiancesrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	irradiancesrvDesc.TextureCube.MipLevels = 1;
	irradiancesrvDesc.TextureCube.MostDetailedMip = 0;

	hr = device->CreateShaderResourceView(IrradiancemapTexture.Get(), &irradiancesrvDesc, IrradianceMapSRV.GetAddressOf());
	
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
	rtvDesc.Texture2DArray.MipSlice = 0;
	rtvDesc.Texture2DArray.ArraySize = 1;

	for (UINT i = 0; i < 6; ++i)
	{
		rtvDesc.Texture2DArray.FirstArraySlice = i;
		hr = device->CreateRenderTargetView(IrradiancemapTexture.Get(), &rtvDesc, irradianceRTVs[i].GetAddressOf());
		if (FAILED(hr)) {
			// handle error
		}
	}

	

	D3D11_TEXTURE2D_DESC irradiancedepthDesc = {};
	irradiancedepthDesc.Width = 32;
	irradiancedepthDesc.Height = 32;
	irradiancedepthDesc.MipLevels = 1;
	irradiancedepthDesc.ArraySize = 1;
	irradiancedepthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	irradiancedepthDesc.SampleDesc.Count = 1;
	irradiancedepthDesc.Usage = D3D11_USAGE_DEFAULT;
	irradiancedepthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = device->CreateTexture2D(&irradiancedepthDesc, nullptr, irradiancedepthStencilBuffer.GetAddressOf());
	hr = device->CreateDepthStencilView(irradiancedepthStencilBuffer.Get(), nullptr, irradiancedepthStencilView.GetAddressOf());
	
	D3D11_DEPTH_STENCIL_DESC IrradiancedepthStencilDesc = {};
	IrradiancedepthStencilDesc.DepthEnable = TRUE;
	IrradiancedepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	IrradiancedepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;


	hr = device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled);

	D3D11_DEPTH_STENCIL_DESC skyboxdepthStencilDesc = {};
	skyboxdepthStencilDesc.DepthEnable = TRUE;
	skyboxdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // 
	skyboxdepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&skyboxdepthStencilDesc, &depthStencilSkyboxState);
	
	
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

	device->CreateTexture2D(&texDesc, nullptr, &PrefilteringTexture);

	hr = device->CreateTexture2D(&pretexDesc, nullptr, PrefilteringTexture.GetAddressOf());
	PrefilteringRTVs.resize(mipLevels);
	for (int i = 0 ; i < PrefilteringRTVs.size(); i++)
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

			if (FAILED(device->CreateRenderTargetView(PrefilteringTexture.Get(), &prertvDesc, PrefilteringRTVs[mip][face].GetAddressOf())))
			{
				assert(false);
			}
		}
	}
	D3D11_SHADER_RESOURCE_VIEW_DESC presrvDesc = {};
	presrvDesc.Format = pretexDesc.Format;
	presrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	presrvDesc.TextureCube.MipLevels = mipLevels;
	presrvDesc.TextureCube.MostDetailedMip = 0;

	device->CreateShaderResourceView(PrefilteringTexture.Get(), &presrvDesc, &PrefilteringSRV);

	D3D11_TEXTURE2D_DESC BRDFDesc = {};
	BRDFDesc.Width = 512; 
	BRDFDesc.Height = 512;
	BRDFDesc.MipLevels = 1;
	BRDFDesc.ArraySize = 1; 
	BRDFDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	BRDFDesc.SampleDesc.Count = 1;
	BRDFDesc.Usage = D3D11_USAGE_DEFAULT;
	BRDFDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;


	hr = device->CreateTexture2D(&BRDFDesc, nullptr, BRDFTexture.GetAddressOf());

		D3D11_RENDER_TARGET_VIEW_DESC BRDFrtvDesc = {};
		BRDFrtvDesc.Format = BRDFDesc.Format;
		BRDFrtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		BRDFrtvDesc.Texture2DArray.MipSlice = 0;
		BRDFrtvDesc.Texture2DArray.ArraySize = 1;

	hr = device->CreateRenderTargetView(BRDFTexture.Get(), &BRDFrtvDesc, &BRDFRTVs);

	D3D11_SHADER_RESOURCE_VIEW_DESC BRDFsrvDesc = {};
	BRDFsrvDesc.Format = BRDFDesc.Format;
	BRDFsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	BRDFsrvDesc.Texture2D.MipLevels = 1;
	
	hr = device->CreateShaderResourceView(BRDFTexture.Get(), &BRDFsrvDesc, &BRDFSRV);

	//CASCADED SHADOW MAPS----------------------------------------
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

		shadowTex.resize(NUM_CASCADES);
		hr = device->CreateTexture2D(&desc, nullptr, shadowTex[i].GetAddressOf());
		shadowDSVs.resize(NUM_CASCADES);
		// Depth view
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		hr = device->CreateDepthStencilView(shadowTex[i].Get(), &dsvDesc, &shadowDSVs[i]);
		shadowSRVs.resize(NUM_CASCADES);
		// SRV for shader access
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(shadowTex[i].Get(), &srvDesc, &shadowSRVs[i]);
	}
	
	D3D11_DEPTH_STENCIL_DESC ShadowdepthStencilDesc = {};
	ShadowdepthStencilDesc.DepthEnable = TRUE;                                 // Enable depth testing
	ShadowdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;        // Allow depth writes
	ShadowdepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;            // Standard comparison

	ShadowdepthStencilDesc.StencilEnable = FALSE;                              // We don’t use stencil here

	 hr = device->CreateDepthStencilState(&ShadowdepthStencilDesc, shadowDepthStencilState.GetAddressOf());


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

	 
	hr = device->CreateSamplerState(&sampDesc, &shadowSampler);
	
	
	
	 D3D11_TEXTURE2D_DESC desc = {};
	 desc.Width = depthMapResolution;
	 desc.Height = depthMapResolution;
	 desc.MipLevels = 1;
	 desc.ArraySize = 1;
	 desc.Format = DXGI_FORMAT_R32_TYPELESS;
	 desc.SampleDesc.Count = 1;
	 desc.Usage = D3D11_USAGE_DEFAULT;
	 desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

	 hr = device->CreateTexture2D(&desc, nullptr, DirectionalshadowTex.GetAddressOf());
	 // Depth view
	 D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	 dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	 dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	 hr = device->CreateDepthStencilView(DirectionalshadowTex.Get(), &dsvDesc, &DirectionalshadowDSVs);
	 // SRV for shader access
	 srvDesc = {};
	 srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	 srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	 srvDesc.Texture2D.MipLevels = 1;
	 hr = device->CreateShaderResourceView(DirectionalshadowTex.Get(), &srvDesc, &DirectionalshadowSRVs);
	
	 D3D11_RASTERIZER_DESC rasterDesc = {};
	 rasterDesc.FillMode = D3D11_FILL_SOLID;
	 rasterDesc.CullMode = D3D11_CULL_FRONT; // Cull front faces
	 rasterDesc.DepthClipEnable = true;

	 
	 device->CreateRasterizerState(&rasterDesc, shadowRasterState.GetAddressOf());
	
	 D3D11_TEXTURE2D_DESC td = {};
	 td.Width = windowWidth / DownSampleMultiplier;
	 td.Height = windowHeight / DownSampleMultiplier;
	 td.MipLevels = 1;
	 td.ArraySize = 1;
	 td.Format = DXGI_FORMAT_R32_FLOAT;
	 td.SampleDesc.Count = 1;
	 td.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	 
	 device->CreateTexture2D(&td, nullptr, RaytracedshadowTex.GetAddressOf());

	 D3D11_UNORDERED_ACCESS_VIEW_DESC uavd = {};
	 uavd.Format = td.Format;
	 uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	 uavd.Texture2D.MipSlice = 0;
	 
	 device->CreateUnorderedAccessView(RaytracedshadowTex.Get(), &uavd, shadowUAV.GetAddressOf());
	
	D3D11_SHADER_RESOURCE_VIEW_DESC rsrv = {};
	rsrv.Format = srvDesc.Format;
	rsrv.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;    // a single 2D map
	rsrv.Texture2D.MipLevels = 1;
	rsrv.Texture2D.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(RaytracedshadowTex.Get(), &srvDesc, RaytracedShadowSRV.GetAddressOf());
	
	D3D11_SHADER_RESOURCE_VIEW_DESC DsrvDesc = {};
	DsrvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS; // Depth only
	DsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	DsrvDesc.Texture2D.MostDetailedMip = 0;
	DsrvDesc.Texture2D.MipLevels = 1;
	hr = device->CreateShaderResourceView(depthStencilBuffer.Get(), &DsrvDesc, DepthBuffer.GetAddressOf());
	
	
	assert(SUCCEEDED(hr));
	return true;




}

void Graphics::CalcCascadeOrthoProjs()
{
	// 1) Inverse of camera view
	XMMATRIX invCamView = XMMatrixInverse(nullptr, camera.GetViewMatrix());

	// 2) Build a "light view" matrix looking along m_LightDir from the origin
	//    (for directional light we treat eye at 0,0,0)
	XMVECTOR eyePos = XMVectorSet(camera.GetPositionFloat3().x, camera.GetPositionFloat3().y, camera.GetPositionFloat3().z,0.0);
	XMVECTOR lightFwd = XMVector3Normalize(XMVectorSet(direction.x, direction.y, direction.z,1.0));
	XMVECTOR lightUp = XMVectorSet(0, 1, 0, 0);
	// if lightDir is (almost) colinear with up, pick another up…
	if (fabsf(XMVectorGetX(XMVector3Dot(lightFwd, lightUp))) > 0.99f)
		lightUp = XMVectorSet(1, 0, 0, 0);
	XMMATRIX lightView = XMMatrixLookAtLH(eyePos, lightFwd, lightUp);

	// 3) Precompute FOV tangents
	//    NOTE: m_CameraProj was built with XMMatrixPerspectiveFovLH(fov, aspect, near, far)
	//    So half-vertical FOV = fov/2, and aspect = width/height.
	float fov = 2.0f * atanf(1.0f / XMVectorGetX(camera.GetProjectionMatrix().r[1])); // recover fov from proj
	float aspect = XMVectorGetX(camera.GetProjectionMatrix().r[1]) / XMVectorGetX(camera.GetProjectionMatrix().r[0]);
	float tanHFOV = tanf(fov * 0.5f);
	float tanVFOV = tanHFOV / aspect;
	m_CascadeLightVP.resize(NUM_CASCADES);
	// 4) For each cascade slice…
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
		m_CascadeLightVP[i] = cascadeProj * lightView;
	}
}
//std::vector<XMFLOAT3> Graphics::getFrustumCornersWorldSpace(const XMMATRIX& proj, const XMMATRIX& view)
//{
//	return GetFrustumCornersWorldSpace( view * proj);
//}

XMMATRIX Graphics::getLightSpaceMatrix(const float nearPlane, const float farPlane)
{
	// 1. Build split projection matrix for the cascade range
	const float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	XMMATRIX splitProj = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(90.0f),
		aspect,
		nearPlane, farPlane);

	XMMATRIX view = camera.GetViewMatrix();
	XMMATRIX viewProj = view * splitProj;

	std::vector<XMVECTOR> corners = getFrustumCornersWorldSpace(viewProj);

	// 2. Compute frustum center
	XMVECTOR center = XMVectorZero();
	for (const auto& pt : corners)
		center += pt;
	center /= static_cast<float>(corners.size());

	// 3. Compute light direction
	float theta = M_PI * Sky.x;
	float phi = 2 * M_PI * Sky.y;
	direction = XMFLOAT3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));

	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&direction));
	XMVECTOR eye = center - lightDir * shadowDirstance;

	XMFLOAT3 up_vec = (Sky.x < -0.1f || Sky.x > -0.9f) ? XMFLOAT3(0, 0, 1) : XMFLOAT3(0, 1, 0);
	XMMATRIX lightView = XMMatrixLookAtLH(eye, center, XMLoadFloat3(&up_vec));

	// 4. Build AABB in light space
	float minX = FLT_MAX, maxX = -FLT_MAX;
	float minY = FLT_MAX, maxY = -FLT_MAX;
	float minZ = FLT_MAX, maxZ = -FLT_MAX;

	for (const auto& corner : corners)
	{
		XMVECTOR trf = XMVector3TransformCoord(corner, lightView);
		XMFLOAT3 pt;
		XMStoreFloat3(&pt, trf);

		minX = std::min(minX, pt.x); maxX = std::max(maxX, pt.x);
		minY = std::min(minY, pt.y); maxY = std::max(maxY, pt.y);
		minZ = std::min(minZ, pt.z); maxZ = std::max(maxZ, pt.z);
	}

	// 5. Snap to texel grid
	const float worldUnitsPerTexel = (maxX - minX) / static_cast<float>(depthMapResolution);
	minX = std::floor(minX / worldUnitsPerTexel) * worldUnitsPerTexel;
	maxX = std::floor(maxX / worldUnitsPerTexel) * worldUnitsPerTexel;
	minY = std::floor(minY / worldUnitsPerTexel) * worldUnitsPerTexel;
	maxY = std::floor(maxY / worldUnitsPerTexel) * worldUnitsPerTexel;

	// 6. Clamp depth
	minZ = std::max(minZ, -farPlane);
	maxZ = std::min(maxZ, farPlane);

	// 7. Final ortho projection
	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(minX, maxX, minY, maxY, minZ, maxZ);
	return XMMatrixTranspose(lightView * lightProj);
}
std::vector<XMMATRIX> Graphics::getLightSpaceMatrices()
{
	std::vector<XMMATRIX> ret;
	for (size_t i = 0; i < NUM_CASCADES; i++)
	{
		if (i == 0)
		{
			ret.push_back(getLightSpaceMatrix(0.01, shadowCascadeLevels[i]));
		}
		else if (i < NUM_CASCADES-1)
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], shadowCascadeLevels[i]));
		}
		else
		{
			ret.push_back(getLightSpaceMatrix(shadowCascadeLevels[i - 1], 50));
		}
	}
	return ret;
}
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