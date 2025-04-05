#include "Gamepch.h"
#include "Application.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

void Application::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	EngineInit::Initialize(hInstance, window_title, window_class, width, height);
	windowWidth = width;
	windowHeight = height;
}

void Application::OnCreate()
{
	InitializeShaders();

	//Initialize Constant Buffer(s)
	HRESULT hr = this->constantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->floorConstantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");


	hr = this->lightConstantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = this->CameraInfoConstantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = m_lightparams.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	this->lightConstantBuffer.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	this->lightConstantBuffer.data.ambientLightStrength = 1.0f;

	HDRIViewProj.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());

	if (!helmet.Initialize("Assets/DamagedHelmet/gLTF/DamagedHelmet.gltf", gfx.GetDevice(), gfx.GetDeviceContext(), this->constantBuffer))
		return;

	/*if (!light.Initialize(gfx.GetDevice(), gfx.GetDeviceContext(), this->constantBuffer))
	{}*/

//	BodyCreationSettings Box_settings(new BoxShape(JPH::Vec3(1, 1, 1)), RVec3(0.0_r, 5.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
//	if (!gameObject.Initialize(gfx.physicsController.CreateAndAddObject(Box_settings, EActivation::Activate),
//	"Assets/TexturedSphere.glb", gfx.GetDevice(), 
//	gfx.GetDeviceContext(), this->constantBuffer))
//{}
	BodyCreationSettings Floor_settings(new BoxShape(JPH::Vec3(100, 0.1, 100)), RVec3(0.0_r, -2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	if (!floor.Initialize(gfx.physicsController.CreateAndAddObject(Floor_settings, EActivation::DontActivate),
		"Assets/Sponza/glTF/Sponza.gltf", gfx.GetDevice(),
		gfx.GetDeviceContext(), this->constantBuffer))
	{
		return;
	}
	
	/*if (!MiscItems.Initialize(
		"Assets/MiscItems.gltf", gfx.GetDevice(),
		gfx.GetDeviceContext(), this->floorConstantBuffer))
	{
		return;
	}*/
	helmet.SetScale({10,10,10});
	floor.SetPosition(XMVECTOR{0.0f,-0.0,0.0});
	floor.SetScale({1,1,1});
	/*MiscItems.SetPosition(XMVECTOR{0.0,-2.0,0.0});
	MiscItems.SetScale({10,10,10});*/
	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 1000000.0f);
	gfx.physicsController.Optimize();

	PlayerCamera.SetPosition(0.0,0.0,-4.0);
	PlayerCamera.SetProjectionValues(90, static_cast<float>(gfx.windowWidth) / static_cast<float>(gfx.windowHeight), 0.1f, 10000.0f);
	//------------------------FullScreenQuad-----------------------------------------//
	std::vector<FullScreenQuad> vertices = {
		// Positions (x, y, z) and Texture coordinates (u, v)
		{DirectX::XMFLOAT2(-1.0f,  1.0f), DirectX::XMFLOAT2(0.0f, 0.0f)}, // Top-left
		{ DirectX::XMFLOAT2(1.0f,  1.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }, // Top-right
		{ DirectX::XMFLOAT2(-1.0f, -1.0f), DirectX::XMFLOAT2(0.0f, 1.0f) }, // Bottom-left
		{ DirectX::XMFLOAT2(1.0f, -1.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }, // Bottom-right
	};

	hr = m_FullScreenVertex.Initialize(gfx.GetDevice(), vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize Vertex buffer.");
	DWORD indices[] = {
	0, 1, 2, // First triangle
	2, 1, 3  // Second triangle
	};
	hr = m_FullScreenIndex.Initialize(gfx.GetDevice(), indices, 6);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");


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

	hr = m_HdriVertex.Initialize(gfx.GetDevice(), cubevertices.data(), cubevertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize Vertex buffer.");
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
	hr = m_HdriIndex.Initialize(gfx.GetDevice(), cubeindices, 36);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");

	viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(gfx.windowWidth);
	viewport.Height = static_cast<float>(gfx.windowHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	gameObject.SetPosition(XMFLOAT3{0,0,0});
	m_lightparams.data.LightColor = DirectX::XMFLOAT3(0.0, 0.0, 100.0);
	m_lightparams.data.LightDirection = DirectX::XMFLOAT3(0.0, 4.0, 0.0);
	m_lightparams.ApplyChanges();


	int width, height, nrComponents;
	float* data = stbi_loadf("Assets/HDRI/kloppenheim_06_puresky_4k.hdr", &width, &height, &nrComponents, 0);
	if (data == nullptr)
	{
		std::cout<<"Failed to load hdri texture" << std::endl;
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

	gfx.device->CreateTexture2D(&texDesc, &initData, gfx.HDRITexture.GetAddressOf());

	stbi_image_free(data);


	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;

	gfx.device->CreateShaderResourceView(gfx.HDRITexture.Get(), &srvDesc, gfx.HDRISRV.GetAddressOf());


}
void Application::InitializeShaders()
{


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BITANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	D3D11_INPUT_ELEMENT_DESC FullScreenRectlayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  }
	};
	
	D3D11_INPUT_ELEMENT_DESC posDesc[]=
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	UINT numElements = ARRAYSIZE(layout);
	if (!m_vertexShader.Initialize(gfx.device, L"CompiledShaders/VertexShader_v.cso", layout, ARRAYSIZE(layout)))
	{
		return;
	}	
	
	if (!m_pixelShader.Initialize(gfx.device, L"CompiledShaders/PixelShader_p.cso"))
	{
		return;
	}

	if (!m_GBuffervertexShader.Initialize(gfx.device, L"CompiledShaders/GBufferVert_v.cso", layout, ARRAYSIZE(layout)))
	{

		return;
	}

	if (!m_GBufferpixelShader.Initialize(gfx.device, L"CompiledShaders/GBufferPixel_p.cso"))
	{
		return;
	}

	if (!m_DeferredvertexShader.Initialize(gfx.device, L"CompiledShaders/DefferedVert_v.cso", FullScreenRectlayout, ARRAYSIZE(FullScreenRectlayout)))
	{

		return;
	}

	if (!m_DeferredpixelShader.Initialize(gfx.device, L"CompiledShaders/DefferedPixel_p.cso"))
	{
		return;
	}
	if (!m_EquiToHDRI_VS.Initialize(gfx.device, L"CompiledShaders/EquiToHdri_v.cso", posDesc, ARRAYSIZE(posDesc)))
	{
		return;
	}
	if (!m_EquiToHdri_PS.Initialize(gfx.device, L"CompiledShaders/EquiToHdri_p.cso"))
	{
		return;
	}

	if (!m_IrradianceConvolution_PS.Initialize(gfx.device, L"CompiledShaders/IrradianceConvolution_p.cso"))
	{
		return;
	}
	if (!m_BackgroundCubemap_VS.Initialize(gfx.device, L"CompiledShaders/BackgroundCubemap_v.cso", posDesc, ARRAYSIZE(posDesc)))
	{
		return;
	}
	if (!m_BackgroundCubemap_PS.Initialize(gfx.device, L"CompiledShaders/BackgroundCubemap_p.cso"))
	{
		return;
	}
}
void Application::OnUpdate()
{
	while (this->ProcessMessages() == true)
	{
		this->gfx.PhysicsUpdate();
		this->RenderFrame();
		float dt = timer.GetMilisecondsElapsed();
		timer.Restart();
		this->Update();
		const float cameraSpeed = 0.1f;
		if (keyboard.KeyIsPressed('W'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed('S'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed('A'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed('D'))
		{
			this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * cameraSpeed * dt);
		}
		if (keyboard.KeyIsPressed(VK_SPACE))
		{
			this->gfx.camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
		}
		if (keyboard.KeyIsPressed('Z'))
		{
			this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
		}
		if (playercam)
		{
			PlayerCamera.SetRadius(5.0f);
			PlayerCamera.HandleInput(MouseX, MouseY);
			data = PlayerCamera.Update(gameObject.GetPositionFloat3());

		}
	}
}


void Application::BindGBufferPass()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float red[] = { 1.0, 0.0, 0.0, 1.0 };
	float green[] = { 0.0, 1.0, 0.0, 1.0 };
	float blue[] = { 0.0, 0.0, 1.0, 1.0 };
 

	std::vector<ID3D11RenderTargetView*> renderTargets = {
	gfx.NormalRTV.Get(),
	gfx.DiffuseRTV.Get(),
	gfx.SpecularRTV.Get(),
	gfx.positionRTV.Get()
	};
	gfx.ClearView(bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());
	gfx.SetInputLayout(this->m_GBuffervertexShader.GetInputLayout());
	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetRasterizerState();
	gfx.SetDepthStencilState();
	gfx.SetBlendState();
	gfx.SetSamplers(); 
	gfx.SetVSShader(m_GBuffervertexShader.GetShader());
	gfx.SetPSShader(m_GBufferpixelShader.GetShader());
	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.GetDeviceContext()->OMSetRenderTargets(renderTargets.size(), renderTargets.data(), gfx.depthStencilView.Get());
	gfx.GetDeviceContext()->ClearRenderTargetView(renderTargets[0], bgcolor);
	gfx.GetDeviceContext()->ClearRenderTargetView(renderTargets[1], bgcolor);
	gfx.GetDeviceContext()->ClearRenderTargetView(renderTargets[2], bgcolor);
	gfx.GetDeviceContext()->ClearRenderTargetView(renderTargets[3], bgcolor);



	RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
	gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	if (playercam)
	{
		{
			this->helmet.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
			floor.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
			//MiscItems.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
		}
	}
	else
	{
		{
			
			this->helmet.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
			floor.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
			//MiscItems.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());

		}
		
	}

}
UINT offset = 0;
void Application::BindLightingPass()
{

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);

	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(),0);
	gfx.SetSamplers();
	gfx.GetDeviceContext()->PSSetSamplers(1, 1, gfx.HDRIsamplerState.GetAddressOf());
	gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());

	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_DeferredvertexShader.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_DeferredpixelShader.GetShader(), NULL, 0);

	std::vector< ID3D11ShaderResourceView*> shaderresources = {
		gfx.NormalSRV.Get(),
		gfx.DiffuseSRV.Get(),
		gfx.SpecularSRV.Get(),
		gfx.positionSRV.Get(),
		gfx.IrradianceMapSRV.Get(),
		gfx.HDRIFramebufferSRV.Get()
	};

	gfx.GetDeviceContext()->PSSetShaderResources(0, shaderresources.size(), shaderresources.data());
	m_lightparams.ApplyChanges();

	gfx.SetPSConstantBuffers(0,1, m_lightparams.GetAddressOf()); 
	CameraInfoConstantBuffer.data.CameraPosition = PlayerCamera.GetPositionFloat3();
	CameraInfoConstantBuffer.data.InvProj = XMMatrixTranspose(XMMatrixInverse(nullptr, camera.GetProjectionMatrix()));
	CameraInfoConstantBuffer.data.InvView = XMMatrixTranspose(XMMatrixInverse(nullptr, camera.GetViewMatrix()));
	CameraInfoConstantBuffer.ApplyChanges();
	gfx.SetPSConstantBuffers(1,1, CameraInfoConstantBuffer.GetAddressOf());
	gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());

	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);
	
	gfx.GetDeviceContext()->DrawIndexed(6,0,0);

	ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
	gfx.GetDeviceContext()->PSSetShaderResources(0, 4, nullSRVs);
	ID3D11SamplerState* nullSampler[1] = { nullptr };
	gfx.GetDeviceContext()->PSSetSamplers(0,1, nullSampler );

}

/*float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gfx.ClearView(bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());
	gfx.SetInputLayout(m_vertexShader.GetInputLayout());
	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetRasterizerState();
	gfx.SetDepthStencilState();
	gfx.SetBlendState();
	gfx.SetSamplers();
	gfx.SetVSShader(m_vertexShader.GetShader());
	gfx.SetPSShader(m_pixelShader.GetShader());
	gfx.SetPSConstantBuffers(0,1, lightConstantBuffer.GetAddressOf());
	gfx.SetPSConstantBuffers(1, 1, CameraInfoConstantBuffer.GetAddressOf());
	this->lightConstantBuffer.data.dynamicLightColor = light.lightColor;
	this->lightConstantBuffer.data.dynamicLightStrength = 1000.0f;
	this->lightConstantBuffer.data.dynamicLightPosition = XMFLOAT3(0.0,1.5,0.0);
	this->lightConstantBuffer.ApplyChanges();

	RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
	gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	if (playercam)
	{
		{
			this->gameObject.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
			floor.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
		}
	}
	else
	{
		{
			this->gameObject.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
			floor.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
		}

	}*/
void Application::DrawHDRI()
{
	D3D11_VIEWPORT hdviewport = {};
	hdviewport.TopLeftX = 0;
	hdviewport.TopLeftY = 0;
	hdviewport.Width = 512;
	hdviewport.Height = 512;
	hdviewport.MinDepth = 0.0f;
	hdviewport.MaxDepth = 1.0f;

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gfx.ClearView(bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());
	gfx.deviceContext->PSSetSamplers(0,1,gfx.HDRIsamplerState.GetAddressOf());

	gfx.SetInputLayout(this->m_EquiToHDRI_VS.GetInputLayout());
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.HDRIdepthStencilStateDisabled.Get(), 0);
	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_EquiToHDRI_VS.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_EquiToHdri_PS.GetShader(), NULL, 0);

	gfx.GetDeviceContext()->PSSetShaderResources(0, 1, gfx.HDRISRV.GetAddressOf());

	gfx.SetVSConstantBuffers(0,1, HDRIViewProj.GetAddressOf());
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
		gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.HDRIFramebufferRTV[i].GetAddressOf(), nullptr);
		gfx.GetDeviceContext()->ClearRenderTargetView(gfx.HDRIFramebufferRTV[i].Get(), bgcolor);
		gfx.GetDeviceContext()->RSSetViewports(1, &hdviewport);

		// Set camera matrices (your constant buffer)
		HDRIViewProj.data.View = XMMatrixTranspose(views[i]);
		HDRIViewProj.data.Projection = XMMatrixTranspose(proj);
		HDRIViewProj.ApplyChanges();
	
		gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
		gfx.GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

		gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
	
	}

	ID3D11RenderTargetView* nullRTVs[1] = { nullptr};
	gfx.GetDeviceContext()->OMSetRenderTargets(1, nullRTVs, nullptr);
	ID3D11SamplerState* nullSampler[1] = { nullptr };
	gfx.GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
	ID3D11Buffer* nullBuffer = nullptr;
	gfx.SetVSConstantBuffers(0, 1, &nullBuffer);

}

void Application::IrradianceConvolution()
{

	D3D11_VIEWPORT irrviewport = {};
	irrviewport.TopLeftX = 0;
	irrviewport.TopLeftY = 0;
	irrviewport.Width = 32;
	irrviewport.Height = 32;
	irrviewport.MinDepth = 0.0f;
	irrviewport.MaxDepth = 1.0f;

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gfx.ClearView(bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());
	gfx.deviceContext->PSSetSamplers(0, 1, gfx.HDRIsamplerState.GetAddressOf());

	gfx.SetInputLayout(this->m_EquiToHDRI_VS.GetInputLayout());
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.ClearDepthStencil(gfx.irradiancedepthStencilView.Get());
	
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.HDRIdepthStencilStateDisabled.Get(), 0);
	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_EquiToHDRI_VS.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_IrradianceConvolution_PS.GetShader(), NULL, 0);

	gfx.GetDeviceContext()->PSSetShaderResources(0, 1, gfx.HDRIFramebufferSRV.GetAddressOf());

	gfx.SetVSConstantBuffers(0, 1, HDRIViewProj.GetAddressOf());
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
		gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.irradianceRTVs[i].GetAddressOf(), gfx.irradiancedepthStencilView.Get());
		gfx.GetDeviceContext()->ClearRenderTargetView(gfx.irradianceRTVs[i].Get(), bgcolor);
		gfx.GetDeviceContext()->RSSetViewports(1, &irrviewport);

		// Set camera matrices (your constant buffer)
		HDRIViewProj.data.View = XMMatrixTranspose(views[i]);
		HDRIViewProj.data.Projection = XMMatrixTranspose(proj);
		HDRIViewProj.ApplyChanges();

		gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
		gfx.GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

		gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);

	}

	ID3D11RenderTargetView* nullRTVs[1] = { nullptr };
	gfx.GetDeviceContext()->OMSetRenderTargets(1, nullRTVs, nullptr);
	ID3D11SamplerState* nullSampler[1] = { nullptr };
	gfx.GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
	ID3D11Buffer* nullBuffer = nullptr;
	gfx.SetVSConstantBuffers(0, 1, &nullBuffer);
}
void Application::Prefiltering()
{

}
void Application::BackgroundCubeMap()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(),nullptr);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.renderTargetView.Get(), bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());

	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilSkyboxState.Get(), 0);
	gfx.GetDeviceContext()->PSSetSamplers(0, 1, gfx.HDRIsamplerState.GetAddressOf());

	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_BackgroundCubemap_VS.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_BackgroundCubemap_PS.GetShader(), NULL, 0);



	gfx.GetDeviceContext()->PSSetShaderResources(0, 1, gfx.HDRIFramebufferSRV.GetAddressOf());
	
	gfx.SetVSConstantBuffers(0,1,HDRIViewProj.GetAddressOf());

	HDRIViewProj.data.Projection = camera.GetProjectionMatrix();
	XMMATRIX view = camera.GetViewMatrix();
	view.r[3] = XMVectorSet(0, 0, 0, 1); // zero translation
	HDRIViewProj.data.View = XMMatrixTranspose(view);
	HDRIViewProj.ApplyChanges();


	gfx.SetInputLayout(this->m_BackgroundCubemap_VS.GetInputLayout());

	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
}

void Application::RenderFrame()
{

	if (RenderIrradianceandHDRI)
	{
		DrawHDRI();
		IrradianceConvolution();
		RenderIrradianceandHDRI = false;

	}
	
	//Square
	
	BindGBufferPass();
	BackgroundCubeMap();
	BindLightingPass();
	//ForwardRender();
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	ImGui::Begin("Light Controls");
	ImGui::DragFloat3("Light direction", &m_lightparams.data.LightDirection.x, 0.1f, 0.0f, 1000.0f);
	ImGui::DragFloat3("Light color ", &m_lightparams.data.LightColor.x, 0.1f, 0.0f, 1000.0f);
	ImGui::Checkbox("Player camera", &playercam);
	
	ImGui::Image((ImTextureID)gfx.positionSRV.Get(), {25,25 });
	ImGui::Image((ImTextureID)gfx.NormalSRV.Get(), { 25,25 });
	ImGui::Image((ImTextureID)gfx.DiffuseSRV.Get(), { 25,25 });
	ImGui::Image((ImTextureID)gfx.SpecularSRV.Get(), { 25,25 });
	/*ImGui::Text("CamPos: %f %f %f", data.campos.x, data.campos.y, data.campos.z);
	ImGui::Text("targetPos: %f %f %f", data.targetpos.x, data.targetpos.y, data.targetpos.z);
	ImGui::Text("x|y|z: %f %f %f", data.x, data.y, data.z);
	ImGui::Text("Pitch|Yaw %f %f", data.pitch, data.yaw);
	ImGui::Text("mouse %f %f", MouseX, MouseY);*/
	//static float Scale[3] = {1.0,1.0,1.0};
	//ImGui::DragFloat3("Scale",&Scale[0], 0.01, 0.0f, 10.0f);
	//floor.SetScale(XMFLOAT3(Scale[0], Scale[1], Scale[2]));
	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
	m_lightparams.ApplyChanges();

	gfx.Present();
}
void Application::ForwardRender()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gfx.ClearView(bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());
	gfx.SetInputLayout(m_vertexShader.GetInputLayout());
	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetRasterizerState();
	gfx.SetDepthStencilState();
	gfx.SetBlendState();
	gfx.SetSamplers();
	gfx.SetVSShader(m_vertexShader.GetShader());
	gfx.SetPSShader(m_pixelShader.GetShader());
	gfx.SetPSConstantBuffers(0, 1, lightConstantBuffer.GetAddressOf());
	gfx.SetPSConstantBuffers(1, 1, CameraInfoConstantBuffer.GetAddressOf());
	this->lightConstantBuffer.data.dynamicLightColor = light.lightColor;
	this->lightConstantBuffer.data.dynamicLightStrength = 1000.0f;
	this->lightConstantBuffer.data.dynamicLightPosition = XMFLOAT3(0.0, 1.5, 0.0);
	this->lightConstantBuffer.ApplyChanges();

	RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
	gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	if (playercam)
	{
		{
			//this->gameObject.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
			floor.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
		}
	}
	else
	{
		{
			//this->gameObject.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
			floor.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
		}

	}
}
void Application::OnUserInput()
{
	float dt = timer.GetMilisecondsElapsed();
	timer.Restart();
	this->Update();
	const float cameraSpeed = 0.001f;
	if (keyboard.KeyIsPressed('W'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetForwardVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('S'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetBackwardVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('A'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetLeftVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed('D'))
	{
		this->gfx.camera.AdjustPosition(this->gfx.camera.GetRightVector() * cameraSpeed * dt);
	}
	if (keyboard.KeyIsPressed(VK_SPACE))
	{
		this->gfx.camera.AdjustPosition(0.0f, cameraSpeed, 0.0f);
	}
	if (keyboard.KeyIsPressed('Z'))
	{
		this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
	}
}

bool Application::playercam = false;

