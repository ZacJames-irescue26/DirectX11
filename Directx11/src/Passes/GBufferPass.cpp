#include "pch.h"
#include "GBufferPass.h"
#include "InputElements.h"

namespace Engine
{

bool GBufferPass::Initialize(ID3D11Device* device)
{
	//GBuffer--------------------------------------------------------------
	// Common settings for G-buffer textures
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Graphics::windowWidth;
	textureDesc.Height = Graphics::windowHeight;
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
	COM_ERROR_IF_FAILED(device->CreateRenderTargetView(NormalTexture.Get(), &renderTargetViewDesc, &NormalRTV), "Failed to create RTV");
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	COM_ERROR_IF_FAILED(device->CreateRenderTargetView(DiffuseTexture.Get(), &renderTargetViewDesc, &DiffuseRTV), "Failed to create RTV");
	renderTargetViewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	COM_ERROR_IF_FAILED(device->CreateRenderTargetView(SpecularTexture.Get(), &renderTargetViewDesc, &SpecularRTV), "Failed to create RTV");

	// Position SRV
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	COM_ERROR_IF_FAILED(device->CreateShaderResourceView(positionTexture.Get(), &srvDesc, &positionSRV), "failed to create SRV");

	srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
	COM_ERROR_IF_FAILED(device->CreateShaderResourceView(NormalTexture.Get(), &srvDesc, &NormalSRV), "failed to create SRV");


	// Specular SRV
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	COM_ERROR_IF_FAILED(device->CreateShaderResourceView(SpecularTexture.Get(), &srvDesc, &SpecularSRV), "failed to create SRV");

	// Diffuse SRV
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	COM_ERROR_IF_FAILED(device->CreateShaderResourceView(DiffuseTexture.Get(), &srvDesc, &DiffuseSRV), "failed to create SRV");



	//Describe our Depth/Stencil Buffer
	D3D11_TEXTURE2D_DESC depthStencilDesc;
	depthStencilDesc.Width = Graphics::windowWidth;
	depthStencilDesc.Height = Graphics::windowHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;


	COM_ERROR_IF_FAILED(device->CreateTexture2D(&depthStencilDesc, NULL, this->depthStencilBuffer.GetAddressOf()), "Failed to create depth stencil buffer.");



	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;   // Stencil-capable view
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	COM_ERROR_IF_FAILED(device->CreateDepthStencilView(depthStencilBuffer.Get(), &dsvDesc, this->depthStencilView.GetAddressOf()), "Failed to create depth stencil view.");

	D3D11_SHADER_RESOURCE_VIEW_DESC depthSrvDesc = {};
	depthSrvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	depthSrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	depthSrvDesc.Texture2D.MostDetailedMip = 0;
	depthSrvDesc.Texture2D.MipLevels = 1;


	COM_ERROR_IF_FAILED(device->CreateShaderResourceView(depthStencilBuffer.Get(), &depthSrvDesc, DepthSRV.GetAddressOf()), "Failed to create SRV");

	if (!m_GBuffervertexShader.Initialize(device, L"CompiledShaders/GBufferVert_v.cso", InputElements::layout, ARRAYSIZE(InputElements::layout)))
	{

		return false;
	}
	if (!m_GBufferAnimatedvertexShader.Initialize(device, L"CompiledShaders/GBufferAnimatedVert_v.cso", InputElements::AnimatedLayout, ARRAYSIZE(InputElements::AnimatedLayout)))
	{

		return false;
	}
	if (!m_GBufferpixelShader.Initialize(device, L"CompiledShaders/GBufferPixel_p.cso"))
	{
		return false;
	}

	return true;
}

void GBufferPass::Draw(Graphics* gfx)
{

}

void GBufferPass::Draw(Graphics* gfx, Scene& scene)
{
	Draw(gfx);
	
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float red[] = { 1.0, 0.0, 0.0, 1.0 };
	float green[] = { 0.0, 1.0, 0.0, 1.0 };
	float blue[] = { 0.0, 0.0, 1.0, 1.0 };
	D3D11_VIEWPORT viewport;
	viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(Graphics::windowWidth);
	viewport.Height = static_cast<float>(Graphics::windowHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	std::vector<ID3D11RenderTargetView*> renderTargets = {
	NormalRTV.Get(),
	DiffuseRTV.Get(),
	SpecularRTV.Get(),
	positionRTV.Get()
	};
	gfx->ClearView(bgcolor);

	gfx->ClearDepthStencil(depthStencilView.Get());
	gfx->SetInputLayout(this->m_GBuffervertexShader.GetInputLayout());
	gfx->SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx->SetRasterizerState();
	gfx->SetDepthStencilState();
	gfx->SetBlendState();
	gfx->SetSamplers();
	gfx->SetVSShader(m_GBuffervertexShader.GetShader());
	gfx->SetPSShader(m_GBufferpixelShader.GetShader());
	gfx->GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx->GetDeviceContext()->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), depthStencilView.Get());
	gfx->GetDeviceContext()->ClearRenderTargetView(renderTargets[0], bgcolor);
	gfx->GetDeviceContext()->ClearRenderTargetView(renderTargets[1], bgcolor);
	gfx->GetDeviceContext()->ClearRenderTargetView(renderTargets[2], bgcolor);
	gfx->GetDeviceContext()->ClearRenderTargetView(renderTargets[3], bgcolor);



	//RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
	//gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	//if (playercam)
	//{
	//	{
	//		this->helmet.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
	//		floor.Draw(PlayerCamera.GetProjectionMatrix() * PlayerCamera.GetViewMatrix());
	//		//MiscItems.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
	//	}
	//}
	//else
	//{
	//	{
			scene.DrawStaticScene(gfx->camera.GetViewMatrix() * gfx->camera.GetProjectionMatrix());
	//	}

	//}
// Animated meshes
			gfx->SetVSShader(m_GBufferAnimatedvertexShader.GetShader());
			gfx->SetPSShader(m_GBufferpixelShader.GetShader());
			gfx->SetInputLayout(m_GBufferAnimatedvertexShader.GetInputLayout());

			scene.DrawAnimatedScene(
				gfx->camera.GetViewMatrix() * gfx->camera.GetProjectionMatrix()
			);

	ID3D11RenderTargetView* nullview[] = { nullptr, nullptr, nullptr, nullptr };
	gfx->GetDeviceContext()->OMSetRenderTargets(4, nullview, nullptr);
	gfx->GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);
	gfx->GetDeviceContext()->VSSetShader(nullptr, nullptr, 0);
}

void GBufferPass::ImGuiPass()
{
	
}

std::vector<ID3D11ShaderResourceView*> GBufferPass::GetSRVRenderTarget()
{
	return {nullptr};
}


}