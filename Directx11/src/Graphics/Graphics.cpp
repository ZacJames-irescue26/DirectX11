#include "pch.h"

#include "Graphics.h"
#include "Vertex.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"

namespace Engine
{
Microsoft::WRL::ComPtr<ID3D11Device> Graphics::device = nullptr;


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
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	//this->deviceContext->ClearRenderTargetView(this->renderTargetView.Get(), bgcolor);
	//this->deviceContext->ClearDepthStencilView(this->depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	//this->deviceContext->IASetInputLayout(this->m_vertexShader.GetInputLayout());
	//this->deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//this->deviceContext->RSSetState(this->rasterizerstate.Get());
	//this->deviceContext->OMSetDepthStencilState(this->depthStencilState.Get(), 0);
	//this->deviceContext->OMSetBlendState(NULL, NULL, 0xFFFFFFFF);
	//this->deviceContext->PSSetSamplers(0, 1, this->samplerState.GetAddressOf());
	//this->deviceContext->VSSetShader(m_vertexShader.GetShader(), NULL, 0);
	//this->deviceContext->PSSetShader(m_pixelShader.GetShader(), NULL, 0);
	//
	//this->deviceContext->PSSetConstantBuffers(0, 1, lightConstantBuffer.GetAddressOf());
	//
	//this->lightConstantBuffer.data.dynamicLightColor = light.lightColor;
	//this->lightConstantBuffer.data.dynamicLightStrength = light.lightStrength;
	//this->lightConstantBuffer.data.dynamicLightPosition = light.GetPositionFloat3();
	//this->lightConstantBuffer.ApplyChanges();
	//
	//RVec3 pos = physicsController.GetPosition(gameObject.GetID());
	//gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	//{
	//	this->gameObject.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
	//	floor.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
	//}

	//Square

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	ImGui::Begin("Light Controls");
	ImGui::DragFloat3("Ambient Light Color", &this->lightConstantBuffer.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Light Strength", &this->lightConstantBuffer.data.ambientLightStrength, 0.01f, 0.0f, 1.0f);
	//static float Scale[3] = {1.0,1.0,1.0};
	//ImGui::DragFloat3("Scale",&Scale[0], 0.01, 0.0f, 10.0f);
	//floor.SetScale(XMFLOAT3(Scale[0], Scale[1], Scale[2]));
	this->lightConstantBuffer.ApplyChanges();
	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	this->swapchain->Present(0, NULL);
}

bool Graphics::InitializeDirectX(HWND hwnd)
{
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
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	hr = this->device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil buffer.");
		return false;
	}

	hr = this->device->CreateDepthStencilView(this->depthStencilBuffer.Get(), NULL, this->depthStencilView.GetAddressOf());
	if (FAILED(hr)) //If error occurred
	{
		ErrorLogger::Log(hr, "Failed to create depth stencil view.");
		return false;
	}
	this->deviceContext->OMSetRenderTargets(1, this->renderTargetView.GetAddressOf(), depthStencilView.Get());
	
	//Create depth stencil state
	D3D11_DEPTH_STENCIL_DESC depthstencildesc;
	ZeroMemory(&depthstencildesc, sizeof(D3D11_DEPTH_STENCIL_DESC));

	depthstencildesc.DepthEnable = true;
	depthstencildesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK::D3D11_DEPTH_WRITE_MASK_ALL;
	depthstencildesc.DepthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;

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
	rasterizerDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
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



	return true;
}

bool Graphics::InitializeShaders()
{
	return true;
}

bool Graphics::InitializeScene()
{

	

	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 1000.0f);
	//physicsController.Optimize();
	//

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
	
	D3D11_DEPTH_STENCIL_DESC HDRIdepthStencilDesc = {};
	HDRIdepthStencilDesc.DepthEnable = TRUE;
	HDRIdepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	HDRIdepthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;


	hr = device->CreateDepthStencilState(&HDRIdepthStencilDesc, &HDRIdepthStencilStateDisabled);
	
	
	
	
	assert(SUCCEEDED(hr));
	return true;




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