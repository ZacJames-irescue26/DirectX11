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

	/*if (!object.Initialize("Assets/TestCube.glb", gfx.GetDevice(), gfx.GetDeviceContext(), this->constantBuffer))
		return;*/

	if (!light.Initialize(gfx.GetDevice(), gfx.GetDeviceContext(), this->constantBuffer))
		return;
	/*if (!floor.Initialize("Assets/Quad.obj", device.Get(), deviceContext.Get(), constantBuffer))
	{
		return false;
	}*/
	BodyCreationSettings Box_settings(new BoxShape(JPH::Vec3(1, 1, 1)), RVec3(0.0_r, 5.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	if (!gameObject.Initialize(gfx.physicsController.CreateAndAddObject(Box_settings, EActivation::Activate),
	"Assets/TexturedSphere.glb", gfx.GetDevice(), 
	gfx.GetDeviceContext(), this->constantBuffer))
		return;
	BodyCreationSettings Floor_settings(new BoxShape(JPH::Vec3(100, 0.1, 100)), RVec3(0.0_r, -2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	/*if (!floor.Initialize(gfx.physicsController.CreateAndAddObject(Floor_settings, EActivation::DontActivate),
		"Assets/Sponza/glTF/sponza.gltf", gfx.GetDevice(),
		gfx.GetDeviceContext(), this->constantBuffer))
		return;*/
	floor.SetPosition(XMVECTOR{0.0f,-2.0,0.0});
	//floor.SetScale({100.0,0.1,100.0});
	camera.SetPosition(0.0f, 0.0f, -2.0f);
	camera.SetProjectionValues(90.0f, static_cast<float>(windowWidth) / static_cast<float>(windowHeight), 0.1f, 10000.0f);
	gfx.physicsController.Optimize();

	PlayerCamera.SetPosition(0.0,0.0,-4.0);
	PlayerCamera.SetProjectionValues(90, static_cast<float>(gfx.windowWidth) / static_cast<float>(gfx.windowHeight), 0.1f, 10000.0f);

	std::vector<FullScreenQuad> vertices = {
		// Positions (x, y, z) and Texture coordinates (u, v)
		{DirectX::XMFLOAT3(-1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f)}, // Top-left
		{ DirectX::XMFLOAT3(1.0f,  1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) }, // Top-right
		{ DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) }, // Bottom-left
		{ DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) }, // Bottom-right
	};

	hr = m_FullScreenVertex.Initialize(gfx.GetDevice(), vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize Vertex buffer.");
	DWORD indices[] = {
	0, 1, 2, // First triangle
	2, 1, 3  // Second triangle
	};
	hr = m_FullScreenIndex.Initialize(gfx.GetDevice(), indices, 6);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer.");

	viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(gfx.windowWidth);
	viewport.Height = static_cast<float>(gfx.windowHeight);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;



}
void Application::InitializeShaders()
{


	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  },
		{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0}

	};

	D3D11_INPUT_ELEMENT_DESC FullScreenRectlayout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA, 0  }
	};

	UINT numElements = ARRAYSIZE(layout);
	if (!m_vertexShader.Initialize(gfx.device, L"CompiledShaders/VertexShader.cso", layout, ARRAYSIZE(layout)))
	{
		return;
	}	
	
	if (!m_pixelShader.Initialize(gfx.device, L"CompiledShaders/PixelShader.cso"))
	{
		return;
	}

	if (!m_GBuffervertexShader.Initialize(gfx.device, L"CompiledShaders/GBufferVert.cso", layout, ARRAYSIZE(layout)))
	{

		return;
	}

	if (!m_GBufferpixelShader.Initialize(gfx.device, L"CompiledShaders/GBufferPixel.cso"))
	{
		return;
	}

	if (!m_DeferredvertexShader.Initialize(gfx.device, L"CompiledShaders/DefferedVert.cso", FullScreenRectlayout, ARRAYSIZE(FullScreenRectlayout)))
	{

		return;
	}

	if (!m_DeferredpixelShader.Initialize(gfx.device, L"CompiledShaders/DefferedPixel.cso"))
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
		const float cameraSpeed = 1.0f;
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


	ID3D11RenderTargetView* renderTargets[] = {
	gfx.NormalRTV.Get(),
	gfx.DiffuseRTV.Get(),
	gfx.SpecularRTV.Get(),
	gfx.positionRTV.Get()
	};

	gfx.GetDeviceContext()->OMSetRenderTargets(4, renderTargets, gfx.depthStencilView.Get());
	//gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.positionRTV.GetAddressOf(), gfx.depthStencilView.Get());
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.NormalRTV.Get(), bgcolor);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.DiffuseRTV.Get(), bgcolor);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.SpecularRTV.Get(), bgcolor);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.positionRTV.Get(), bgcolor);
	//gfx.ClearDepthStencil(gfx.depthStencilView.Get());

	gfx.SetRasterizerState();
	gfx.SetDepthStencilState();
	gfx.SetBlendState();
	gfx.SetSamplers();
	gfx.SetInputLayout(this->m_GBuffervertexShader.GetInputLayout());
	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetVSShader(m_GBuffervertexShader.GetShader());
	gfx.SetPSShader(m_GBufferpixelShader.GetShader());


	RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
	//gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	if (playercam)
	{
		
			this->gameObject.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
			//floor.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
		
	}
	else
	{
		
			this->gameObject.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
			//floor.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
		
		
	}


	ID3D11RenderTargetView* nullRTV[4] = {nullptr, nullptr, nullptr, nullptr};
	gfx.GetDeviceContext()->OMSetRenderTargets(4, nullRTV, nullptr);

}
UINT offset = 0;
void Application::BindLightingPass()
{

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.renderTargetView.Get(), bgcolor);
	gfx.ClearDepthStencil(gfx.depthStencilView.Get());
	gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());

	

	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_vertexShader.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_pixelShader.GetShader(), NULL, 0);

	gfx.GetDeviceContext()->PSSetShaderResources(0, 1, gfx.NormalSRV.GetAddressOf());
	gfx.GetDeviceContext()->PSSetShaderResources(1, 1, gfx.DiffuseSRV.GetAddressOf());
	gfx.GetDeviceContext()->PSSetShaderResources(2, 1, gfx.SpecularSRV.GetAddressOf());
	gfx.GetDeviceContext()->PSSetShaderResources(3, 1, gfx.positionSRV.GetAddressOf());

	RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
	gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	
		this->gameObject.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
		//floor.Draw(camera.GetViewMatrix() * camera.GetProjectionMatrix());
	


	m_lightparams.data.LightColor = DirectX::XMFLOAT3(1.0, 1.0, 1.0);
	m_lightparams.data.LightDirection = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
	m_lightparams.data.LightPos = DirectX::XMFLOAT3(0.0, 0.0, 0.0);
	m_lightparams.data.LightRange = DirectX::XMFLOAT4(5, 5, 5, 5);
	m_lightparams.ApplyChanges();
	
	gfx.SetPSConstantBuffers(0,1, m_lightparams.GetAddressOf());

	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);
	
	gfx.GetDeviceContext()->DrawIndexed(6,0,0);

	ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
	gfx.GetDeviceContext()->PSSetShaderResources(0, 4, nullSRVs);

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

void Application::RenderFrame()
{
	CameraInfoConstantBuffer.data.CameraPosition = PlayerCamera.GetPositionFloat3();
	CameraInfoConstantBuffer.ApplyChanges();
	
	
	//Square
	BindGBufferPass();
	BindLightingPass();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	ImGui::Begin("Light Controls");
	ImGui::DragFloat3("Ambient Light Color", &this->lightConstantBuffer.data.ambientLightColor.x, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat("Ambient Light Strength", &this->lightConstantBuffer.data.ambientLightStrength, 0.01f, 0.0f, 1000.0f);
	ImGui::Checkbox("Player camera", &playercam);
	/*ImGui::Text("CamPos: %f %f %f", data.campos.x, data.campos.y, data.campos.z);
	ImGui::Text("targetPos: %f %f %f", data.targetpos.x, data.targetpos.y, data.targetpos.z);
	ImGui::Text("x|y|z: %f %f %f", data.x, data.y, data.z);
	ImGui::Text("Pitch|Yaw %f %f", data.pitch, data.yaw);
	ImGui::Text("mouse %f %f", MouseX, MouseY);*/

	//static float Scale[3] = {1.0,1.0,1.0};
	//ImGui::DragFloat3("Scale",&Scale[0], 0.01, 0.0f, 10.0f);
	//floor.SetScale(XMFLOAT3(Scale[0], Scale[1], Scale[2]));
	this->lightConstantBuffer.ApplyChanges();
	ImGui::End();
	//Assemble Together Draw Data
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	gfx.Present();
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

