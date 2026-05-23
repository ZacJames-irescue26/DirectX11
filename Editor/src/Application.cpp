#include "Gamepch.h"
#include "Application.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include <format>
#include "InputElements.h"


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




	

	



	
	hr = m_ViewProj.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = m_DebugColors.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = m_CastLight.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = lcb.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	hr = m_BaseCB.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	m_SurfelCSBUffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");


	m_CastLight.data.light.position = XMFLOAT3(0.0,5.0,0.0);
	m_CastLight.data.light.intensity = XMFLOAT3(1.0, 1.0, 1.0);
	m_CastLight.data.light.direction = XMFLOAT3(0.0, -1.0, 0.0);
	m_CastLight.data.light.cutOff = 0.9;
	m_CastLight.ApplyChanges();

	this->lightConstantBuffer.data.ambientLightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
	this->lightConstantBuffer.data.ambientLightStrength = 1.0f;

	HDRIViewProj.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());

	m_Scene = std::make_unique<Scene>();

	auto sponza = m_Scene->AddEntity("Sponza");

	sponza->AddComponent(std::make_unique<TransformComponent>(XMFLOAT3{0.0,-1.0,0.0},XMFLOAT3{0.0,0.0,0.0},XMFLOAT3{0.1,0.1,0.1}));
	sponza->AddComponent(std::make_unique<StaticMeshComponent>("Assets/Sponza/glTF/Sponza.gltf", gfx.GetDevice(), gfx.GetDeviceContext(), constantBuffer));



	//if (!helmet.Initialize("Assets/DamagedHelmet/gLTF/DamagedHelmet.gltf", gfx.GetDevice(), gfx.GetDeviceContext(), this->constantBuffer))
	//	return;
	

	/*if (!light.Initialize(gfx.GetDevice(), gfx.GetDeviceContext(), this->constantBuffer))
	{}*/

//	BodyCreationSettings Box_settings(new BoxShape(JPH::Vec3(1, 1, 1)), RVec3(0.0_r, 5.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
//	if (!gameObject.Initialize(gfx.physicsController.CreateAndAddObject(Box_settings, EActivation::Activate),
//	"Assets/TexturedSphere.glb", gfx.GetDevice(), 
//	gfx.GetDeviceContext(), this->constantBuffer))
//{}
// 
// 
// 
	//BodyCreationSettings Floor_settings(new BoxShape(JPH::Vec3(100, 0.1, 100)), RVec3(0.0_r, -2.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);

	//if (!floor.Initialize(gfx.physicsController.CreateAndAddObject(Floor_settings, EActivation::DontActivate),
	//	"Assets/Sponza/glTF/Sponza.gltf", gfx.GetDevice(),
	//	gfx.GetDeviceContext(), this->constantBuffer))
	//{
	//	return;
	//}


	/*if (!MiscItems.Initialize(
		"Assets/MiscItems.gltf", gfx.GetDevice(),
		gfx.GetDeviceContext(), this->floorConstantBuffer))
	{
		return;
	}*/
	//helmet.SetScale({0.1,0.1,0.1});
	//helmet.SetPosition(XMFLOAT3{1.0,1.0,1.0});
	//floor.SetPosition(XMVECTOR{-.1f,-0.1,-0.1});
	//floor.SetScale({0.01,0.01,0.01});
	/*MiscItems.SetPosition(XMVECTOR{0.0,-2.0,0.0});
	MiscItems.SetScale({10,10,10});*/
	//gfx.physicsController.Optimize();
	//PlayerCamera.SetPosition(0.0,0.0,-4.0);
	//PlayerCamera.SetProjectionValues(90, static_cast<float>(gfx.windowWidth) / static_cast<float>(gfx.windowHeight), 0.1f, 1000.0f);
	
	gfx.camera.SetPosition(0.0,3.0,0.0);
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



	/*gameObject.SetPosition(XMFLOAT3{0,0,0});
	m_lightparams.data.LightColor = DirectX::XMFLOAT3(10.0, 10.0, 10.0);
	m_lightparams.data.LightDirection = DirectX::XMFLOAT3(0.0, -4.8, -1.0);
	m_lightparams.ApplyChanges();*/




	//AABB testbox = AABB(XMFLOAT3{-15.0,-5.0,-15.0},{15.0,15.0,15.0});
	//octree = new Octree(&testbox,5);
	//std::vector<GameObject> objects;
	//objects.push_back(floor);

	//accel = BVHBuilder::BuildModelAccel(floor.GetModel(), 500);

	//
	//
	//BVHBuilder::FlattenMeshBVH(accel.get(), Flat, triangles, baseRoots, floor.worldMatrix);

	//D3D11_BUFFER_DESC bufDesc = {};
	//bufDesc.ByteWidth = UINT(Flat.size() * sizeof(FlatNode));
	//bufDesc.Usage = D3D11_USAGE_DEFAULT;
	//bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//bufDesc.CPUAccessFlags = 0;
	//bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//bufDesc.StructureByteStride = sizeof(FlatNode);

	//// 2) Provide initial data
	//D3D11_SUBRESOURCE_DATA flatdata = {};
	//flatdata.pSysMem = Flat.data();
	//// 3) Create the buffer

	// hr = gfx.device->CreateBuffer(&bufDesc, &flatdata, nodeBuffer.GetAddressOf());
	//// check hr...

	//// 4) Create the SRV so shaders can read it
	//D3D11_SHADER_RESOURCE_VIEW_DESC flatdesc = {};
	//flatdesc.Format = DXGI_FORMAT_UNKNOWN;  // structured buffer
	//flatdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//flatdesc.Buffer.ElementOffset = 0;
	//flatdesc.Buffer.NumElements = UINT(Flat.size());


	//hr = gfx.device->CreateShaderResourceView(nodeBuffer.Get(), &flatdesc, nodeSRV.GetAddressOf());
	//
	//bufDesc.ByteWidth = UINT(triangles.size() * sizeof(TriangleJustPos));
	//bufDesc.Usage = D3D11_USAGE_DEFAULT;
	//bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//bufDesc.CPUAccessFlags = 0;
	//bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//bufDesc.StructureByteStride = sizeof(TriangleJustPos);

	//// 2) Provide initial data
	//flatdata.pSysMem = triangles.data();

	//// 3) Create the buffer

	// hr = gfx.device->CreateBuffer(&bufDesc, &flatdata, TrianglesBuffer.GetAddressOf());
	//// check hr...

	//// 4) Create the SRV so shaders can read it
	//flatdesc.Format = DXGI_FORMAT_UNKNOWN;  // structured buffer
	//flatdesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//flatdesc.Buffer.ElementOffset = 0;
	//flatdesc.Buffer.NumElements = UINT(triangles.size());


	//hr = gfx.device->CreateShaderResourceView(TrianglesBuffer.Get(), &flatdesc, TrianglesSRV.GetAddressOf());
	//// create StructuredBuffer<uint> for baseRoots
	//D3D11_BUFFER_DESC bdesc = {};
	//bdesc.ByteWidth = UINT(baseRoots.size() * sizeof(uint32_t));
	//bdesc.Usage = D3D11_USAGE_DEFAULT;
	//bdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	//bdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	//bdesc.StructureByteStride = sizeof(uint32_t);

	//D3D11_SUBRESOURCE_DATA init = { baseRoots.data(), 0, 0 };

	//
	//gfx.device->CreateBuffer(&bdesc, &init, &baseRootBuffer);

	//D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	//srvd.Format = DXGI_FORMAT_UNKNOWN;
	//srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	//srvd.Buffer.NumElements = UINT(baseRoots.size());

	//
	//gfx.device->CreateShaderResourceView(
	//	baseRootBuffer.Get(), &srvd, &baseRootSRV);
	D3D11_BUFFER_DESC surfelBufDesc = {};
	surfelBufDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
	surfelBufDesc.ByteWidth = sizeof(GPUSurfel) * 10000;
	surfelBufDesc.StructureByteStride = sizeof(GPUSurfel);
	surfelBufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	surfelBufDesc.Usage = D3D11_USAGE_DEFAULT;
	surfelBufDesc.CPUAccessFlags = 0;

	
	hr = gfx.device->CreateBuffer(&surfelBufDesc, nullptr, m_SurfelsBuffer.GetAddressOf());

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = 10000;

	
	hr = gfx.device->CreateUnorderedAccessView(m_SurfelsBuffer.Get(), &uavDesc, m_SurfelsUAV.GetAddressOf());

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(GPUSurfel) * 10000;
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	vbDesc.StructureByteStride = 0;
	vbDesc.MiscFlags = 0;

	
	gfx.device->CreateBuffer(&vbDesc, nullptr, m_VertexBuffer.GetAddressOf());


	D3D11_BUFFER_DESC bdesc = {};
	bdesc.ByteWidth = 4; // size of a single uint
	bdesc.Usage = D3D11_USAGE_DEFAULT;
	bdesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bdesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
	bdesc.StructureByteStride = 0;

	
	hr = gfx.device->CreateBuffer(&bdesc, nullptr, surfelCounterBuffer.GetAddressOf());

	D3D11_UNORDERED_ACCESS_VIEW_DESC suavDesc = {};
	suavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	suavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	suavDesc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;
	suavDesc.Buffer.NumElements = 1;
	suavDesc.Buffer.FirstElement = 0;

	
	hr = gfx.device->CreateUnorderedAccessView(surfelCounterBuffer.Get(), &suavDesc, surfelCounterUAV.GetAddressOf());

	UINT zero = 0;
	gfx.deviceContext->UpdateSubresource(surfelCounterBuffer.Get(), 0, nullptr, &zero, 0, 0);

	D3D11_TEXTURE2D_DESC ttexDesc = {};
	ttexDesc.Width = windowWidth / 16;
	ttexDesc.Height = windowHeight / 16;
	ttexDesc.MipLevels = 1;
	ttexDesc.ArraySize = 1;
	ttexDesc.Format = DXGI_FORMAT_R32_UINT;
	ttexDesc.SampleDesc.Count = 1;
	ttexDesc.Usage = D3D11_USAGE_DEFAULT;
	ttexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	ttexDesc.CPUAccessFlags = 0;
	ttexDesc.MiscFlags = 0;

	
	hr = gfx.device->CreateTexture2D(&ttexDesc, nullptr, m_TileCoverageTex.GetAddressOf());

	D3D11_UNORDERED_ACCESS_VIEW_DESC tuavDesc = {};
	tuavDesc.Format = DXGI_FORMAT_R32_UINT;
	tuavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	tuavDesc.Texture2D.MipSlice = 0;

	
	hr = gfx.device->CreateUnorderedAccessView(m_TileCoverageTex.Get(), &tuavDesc, m_TileCoverageUAV.GetAddressOf());


	UINT clearValue[4] = { 0, 0, 0, 0 };
	gfx.deviceContext->ClearUnorderedAccessViewUint(m_TileCoverageUAV.Get(), clearValue);

	D3D11_SHADER_RESOURCE_VIEW_DESC tsrvDesc = {};
	tsrvDesc.Format = DXGI_FORMAT_R32_UINT;
	tsrvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	tsrvDesc.Texture2D.MostDetailedMip = 0;
	tsrvDesc.Texture2D.MipLevels = 1;

	
	hr = gfx.device->CreateShaderResourceView(m_TileCoverageTex.Get(), &tsrvDesc, m_TileCoverageSRV.GetAddressOf());

	m_BRDFPass.Initialize(gfx.device.Get());
	m_HDRIPass.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_GBuffer.Initialize(gfx.GetDevice());
	m_IrradianceConvolution.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_LightingPass.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_Prefiltering.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_ShadowPass.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());

	m_SceneHierarchyPanel = std::make_unique<Editor::SceneHierarchyPanel>(m_Scene.get(), gfx.device.Get(), gfx.deviceContext.Get(), &floorConstantBuffer);



}

void Application::InitializeShaders()
{


	

	//UINT numElements = ARRAYSIZE(layout);
	//if (!m_vertexShader.Initialize(gfx.device, L"CompiledShaders/VertexShader_v.cso", layout, ARRAYSIZE(layout)))
	//{
	//	return;
	//}	
	//
	//if (!m_pixelShader.Initialize(gfx.device, L"CompiledShaders/PixelShader_p.cso"))
	//{
	//	return;
	//}







	if (!m_BackgroundCubemap_VS.Initialize(gfx.device.Get(), L"CompiledShaders/BackgroundCubemap_v.cso", InputElements::posDesc, ARRAYSIZE(InputElements::posDesc)))
	{
		return;
	}
	if (!m_BackgroundCubemap_PS.Initialize(gfx.device.Get(), L"CompiledShaders/BackgroundCubemap_p.cso"))
	{
		return;
	}


	//if (!m_DebugCascade_VS.Initialize(gfx.device, L"CompiledShaders/DebugCascade_v.cso", ModelPos, ARRAYSIZE(ModelPos)))
	//{
	//	return;
	//}
	//if (!m_DebugCascade_PS.Initialize(gfx.device, L"CompiledShaders/DebugCascade_p.cso"))
	//{
	//	return;
	//}
	//if (!m_DebugDrawShadowMap_PS.Initialize(gfx.device, L"CompiledShaders/DrawShadowMap_p.cso"))
	//{
	//	return;
	//}

	//if (!m_SurfelDebug_VS.Initialize(gfx.device, L"CompiledShaders/SurfelDebug_v.cso", Surfelvb, ARRAYSIZE(Surfelvb)))
	//{
	//	return;
	//}
	//if (!m_SurfelDebug_PS.Initialize(gfx.device, L"CompiledShaders/SurfelDebug_p.cso"))
	//{
	//	return;
	//}
	//if (!m_SureflDebug_GS.Initialize(gfx.device, L"CompiledShaders/SurfelDebug_g.cso"))
	//{
	//	return;
	//}
	//if (!m_Shadow_CS.Initialize(gfx.device, L"CompiledShaders/RealtimeShadows_c.cso"))
	//{
	//	return;
	//}
	//if (!m_GenerateSurfel_CS.Initialize(gfx.device, L"CompiledShaders/GenerateSurfels_c.cso"))
	//{
	//	return;
	//}

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
		const float cameraSpeed = 0.01f;
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
			//PlayerCamera.SetRadius(5.0f);
			//PlayerCamera.HandleInput(MouseX, MouseY);
			//data = PlayerCamera.Update(gameObject.GetPositionFloat3());

		}
	}
}


void Application::BindGBufferPass()
{
	
}
UINT offset = 0;
void Application::RenderToRaytraceToSRV()
{
	//float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	//gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);

	//gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	//gfx.SetRasterizerState();
	//gfx.SetBlendState();
	//gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(), 0);
	//gfx.SetSamplers();
	//gfx.GetDeviceContext()->PSSetSamplers(1, 1, gfx.HDRIsamplerState.GetAddressOf());
	//gfx.GetDeviceContext()->PSSetSamplers(2, 1, gfx.shadowSampler.GetAddressOf());
	//gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());

	//gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//gfx.GetDeviceContext()->VSSetShader(m_DeferredvertexShader.GetShader(), NULL, 0);
	//gfx.GetDeviceContext()->PSSetShader(m_DeferredpixelShader.GetShader(), NULL, 0);

	//std::vector< ID3D11ShaderResourceView*> shaderresources = {
	//	gfx.NormalSRV.Get(),
	//	gfx.DiffuseSRV.Get(),
	//	gfx.SpecularSRV.Get(),
	//	gfx.positionSRV.Get(),
	//	gfx.IrradianceMapSRV.Get(),
	//	gfx.HDRIFramebufferSRV.Get(),
	//	gfx.PrefilteringSRV.Get(),
	//	gfx.BRDFSRV.Get(),
	//	gfx.DirectionalshadowSRVs.Get(),
	//};

	//gfx.GetDeviceContext()->PSSetShaderResources(0, shaderresources.size(), shaderresources.data());

	//m_lightparams.data.LightSpaceMatrices = lightMatrices;
	//m_lightparams.data.farPlane = 1000;
	//m_lightparams.ApplyChanges();

	//gfx.SetPSConstantBuffers(0, 1, m_lightparams.GetAddressOf());
	//CameraInfoConstantBuffer.data.CameraPosition = PlayerCamera.GetPositionFloat3();
	//CameraInfoConstantBuffer.data.InvProj = XMMatrixTranspose(XMMatrixInverse(nullptr, camera.GetProjectionMatrix()));
	//CameraInfoConstantBuffer.data.InvView = XMMatrixTranspose(XMMatrixInverse(nullptr, camera.GetViewMatrix()));
	//CameraInfoConstantBuffer.data.View = XMMatrixTranspose(camera.GetViewMatrix());
	//CameraInfoConstantBuffer.ApplyChanges();
	//gfx.SetPSConstantBuffers(1, 1, CameraInfoConstantBuffer.GetAddressOf());
	//gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());
	//gfx.SetPSConstantBuffers(2, 1, m_CastLight.GetAddressOf());
	//m_CastLight.ApplyChanges();


	//gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
	//gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	//gfx.GetDeviceContext()->DrawIndexed(6, 0, 0);

	//ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
	//gfx.GetDeviceContext()->PSSetShaderResources(0, 4, nullSRVs);
	//ID3D11SamplerState* nullSampler[1] = { nullptr };
	//gfx.GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);

}
void Application::BindLightingPass()
{


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
//void Application::DrawHDRI()
//{
//
//
//}
//
//void Application::IrradianceConvolution()
//{
//
//	
//}
//void Application::Prefiltering()
//{
//
//	
//}
//void Application::BRDF()
//{
//
//
//
//
//}
//void Application::BackgroundCubeMap()
//{
//	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(),nullptr);
//	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.renderTargetView.Get(), bgcolor);
//
//	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
//	gfx.SetRasterizerState();
//	gfx.SetBlendState();
//	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilSkyboxState.Get(), 0);
//	gfx.GetDeviceContext()->PSSetSamplers(0, 1, gfx.HDRIsamplerState.GetAddressOf());
//
//	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	gfx.GetDeviceContext()->VSSetShader(m_BackgroundCubemap_VS.GetShader(), NULL, 0);
//	gfx.GetDeviceContext()->PSSetShader(m_BackgroundCubemap_PS.GetShader(), NULL, 0);
//
//
//	gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//
//	gfx.GetDeviceContext()->PSSetShaderResources(0, 1, gfx.HDRIFramebufferSRV.GetAddressOf());
//	
//	gfx.SetVSConstantBuffers(0,1,HDRIViewProj.GetAddressOf());
//
//	HDRIViewProj.data.Projection = gfx.camera.GetProjectionMatrix();
//	XMMATRIX view = gfx.camera.GetViewMatrix();
//	view.r[3] = XMVectorSet(0, 0, 0, 1); // zero translation
//	HDRIViewProj.data.View = XMMatrixTranspose(view);
//	HDRIViewProj.ApplyChanges();
//
//
//	gfx.SetInputLayout(this->m_BackgroundCubemap_VS.GetInputLayout());
//
//	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
//	gfx.GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//	gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
//}
//
//void Application::ShadowDepthPass()
//{
//	
//}
//void Application::DrawDebugCascade()
//{
//	DWORD cascadeindices[] = {
//		0, 2, 3,
//		0, 3, 1,
//		4, 6, 2,
//		4, 2, 0,
//		5, 7, 6,
//		5, 6, 4,
//		1, 3, 7,
//		1, 7, 5,
//		6, 7, 3,
//		6, 3, 2,
//		1, 5, 4,
//		0, 1, 4
//	};
//	m_DebugCascade.resize(8);
//	for (int i = 0; i < gfx.NUM_CASCADES; i++)
//	{
//		const auto corners = gfx.getFrustumCornersWorldSpace(gfx.m_CascadeLightVP[i]);
//		std::vector<CubeWPos> vec3s;
//		vec3s.resize(corners.size());
//		for (int i = 0; i < corners.size(); i++)
//		{
//			XMFLOAT3 pos;
//			XMStoreFloat3(&pos, corners[i]);
//			vec3s[i].pos = pos;
//		}
//
//		m_DebugCascade[i].Initialize(gfx.device.Get(), vec3s.data(), vec3s.size());
//
//	}
//	m_CascadeIndex.Initialize(gfx.device.Get(), cascadeindices, 36);
//	float blendFactor[4] = { 0, 0, 0, 0 }; // usually ignored unless BlendFactor used
//	UINT sampleMask = 0xffffffff;
//	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//	gfx.SetInputLayout(m_DebugCascade_VS.GetInputLayout());
//	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	gfx.SetDepthStencilState();
//	gfx.GetDeviceContext()->OMSetBlendState(gfx.transparentBlendState.Get(),blendFactor, sampleMask);
//	gfx.SetSamplers();
//	gfx.SetVSShader(m_DebugCascade_VS.GetShader());
//	gfx.SetPSShader(m_DebugCascade_PS.GetShader());
//
//	gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ViewProj.GetAddressOf());
//
//	m_ViewProj.data.View = XMMatrixTranspose(gfx.camera.GetViewMatrix());
//	m_ViewProj.data.Projection = XMMatrixTranspose(gfx.camera.GetProjectionMatrix());
//	m_ViewProj.ApplyChanges();
//	gfx.GetDeviceContext()->PSSetConstantBuffers(0,1, m_DebugColors.GetAddressOf());
//	gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//
//
//	for (int i = 0; i < gfx.NUM_CASCADES; i++)
//	{
//		m_DebugColors.data.index = i;
//		m_DebugColors.ApplyChanges();
//		gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_DebugCascade[i].GetAddressOf(), this->m_DebugCascade[i].StridePtr(), &offset);
//		gfx.GetDeviceContext()->IASetIndexBuffer(m_CascadeIndex.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//		gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
//	}
//
//}
//void Application::DrawShadowMaps()
//{
//	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//
//	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
//	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.renderTargetView.Get(), clearColor);
//
//	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
//	gfx.SetRasterizerState();
//	gfx.SetBlendState();
//	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(), 0);
//	gfx.SetSamplers();
//	gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());
//
//	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	gfx.GetDeviceContext()->VSSetShader(m_DeferredvertexShader.GetShader(), NULL, 0);
//	gfx.GetDeviceContext()->PSSetShader(m_DebugDrawShadowMap_PS.GetShader(), NULL, 0);
//
//
//	gfx.GetDeviceContext()->PSSetShaderResources(0, 1,gfx.DirectionalshadowSRVs.GetAddressOf());
//
//	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
//	gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//	gfx.GetDeviceContext()->DrawIndexed(6, 0, 0);
//}
//void Application::DirectionalShadowMap()
//{
//
//	
//
//	
//}
//void Application::DrawSurfels()
//{
//	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//	gfx.deviceContext->CopyResource(m_VertexBuffer.Get(), m_SurfelsBuffer.Get());
//	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
//	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(), 0);
//	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
//	gfx.SetBlendState();
//	gfx.SetDepthStencilState();
//	gfx.SetInputLayout(m_SurfelDebug_VS.GetInputLayout());
//	gfx.SetRasterizerState();
//	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
//	gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ViewProj.GetAddressOf());
//	gfx.GetDeviceContext()->GSSetConstantBuffers(0, 1, m_ViewProj.GetAddressOf());
//	gfx.deviceContext->VSSetShader(m_SurfelDebug_VS.GetShader(), nullptr, 0);
//	m_ViewProj.data.Projection = XMMatrixTranspose(gfx.camera.GetProjectionMatrix());
//	m_ViewProj.data.View = XMMatrixTranspose(gfx.camera.GetViewMatrix());
//	m_ViewProj.ApplyChanges();
//	const UINT stride = sizeof(SurfelVB);
//	gfx.deviceContext->IASetVertexBuffers(0,1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
//	gfx.SetPSShader(m_SurfelDebug_PS.GetShader());
//	//gfx.GetDeviceContext()->GSSetShader(m_SureflDebug_GS.GetShader(), nullptr, 0);
//
//	gfx.GetDeviceContext()->Draw(10000, 0);
//	gfx.GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);
//
//}
//void Application::SpawnSurfels()
//{ 
//	gfx.deviceContext->CSSetShader(m_GenerateSurfel_CS.GetShader(), nullptr, 0);
//	UINT clearValue[4] = { 0, 0, 0, 0 };
//	//gfx.deviceContext->ClearUnorderedAccessViewUint(m_TileCoverageUAV.Get(), clearValue);
//	//GBuffer
//	gfx.deviceContext->CSSetShaderResources(0, 1, gfx.positionSRV.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(1, 1, gfx.NormalSRV.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(2, 1, gfx.DiffuseSRV.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(3, 1, gfx.DepthBuffer.GetAddressOf());
//	gfx.deviceContext->CSSetSamplers(0,1, gfx.samplerState.GetAddressOf());
//
//	//Surfels
//	gfx.deviceContext->CSSetUnorderedAccessViews(0, 1, m_SurfelsUAV.GetAddressOf(), nullptr);
//	gfx.deviceContext->CSSetUnorderedAccessViews(1, 1, surfelCounterUAV.GetAddressOf(), nullptr);
//	gfx.deviceContext->CSSetUnorderedAccessViews(2, 1, m_TileCoverageUAV.GetAddressOf(), nullptr);
//	gfx.deviceContext->CSSetConstantBuffers(0,1, m_SurfelCSBUffer.GetAddressOf());
//	gfx.deviceContext->CSSetConstantBuffers(1,1,m_ObjectModel.GetAddressOf());
//	m_SurfelCSBUffer.data.screenWidth = windowWidth;
//	m_SurfelCSBUffer.data.screenHeight = windowHeight;
//	m_SurfelCSBUffer.data.tileCountX = windowWidth / 16;
//	m_SurfelCSBUffer.data.tileCountY = windowHeight / 16;
//	m_SurfelCSBUffer.data.viewProj = XMMatrixTranspose(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
//	m_SurfelCSBUffer.data.screenSize = { (float)windowWidth, (float)windowHeight };
//
//	m_SurfelCSBUffer.ApplyChanges();
//
//	XMMATRIX proj = gfx.camera.GetProjectionMatrix();
//	XMMATRIX view = gfx.camera.GetViewMatrix();
//
//	// ViewProj = proj * view
//	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
//
//	// InvViewProj = inverse(ViewProj)
//	XMMATRIX invViewProj = XMMatrixInverse(nullptr, viewProj);
//
//	// Transpose for HLSL row-major convention
//	m_ObjectModel.data.Model = XMMatrixTranspose(invViewProj);
//	m_ObjectModel.ApplyChanges();
//	gfx.deviceContext->Dispatch((windowWidth+15)/16, (windowHeight+15)/16, 1);
//
//
//
//
//	ID3D11ShaderResourceView* nullSRVs = nullptr;
//	ID3D11SamplerState* nullSampler[1] = { nullptr };
//	gfx.GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
//	gfx.deviceContext->CSSetShaderResources(0, 1, &nullSRVs);
//	gfx.deviceContext->CSSetShaderResources(1, 1, &nullSRVs);
//	gfx.deviceContext->CSSetShaderResources(1, 1, &nullSRVs);
//	gfx.deviceContext->CSSetSamplers(0, 1, nullSampler);
//
//	ID3D11UnorderedAccessView* nullUAV = nullptr;
//	gfx.deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
//	
//	gfx.deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
//	gfx.deviceContext->CSSetUnorderedAccessViews(1, 1, &nullUAV, nullptr);
//	gfx.deviceContext->CSSetUnorderedAccessViews(2, 1, &nullUAV, nullptr);
//
//}
//
//void Application::UpdateSurfels()
//{
//
//}

//void Application::RayTraceShadows()
//{
//
//	gfx.deviceContext->CSSetShader(m_Shadow_CS.GetShader(), nullptr, 0);
//	lcb.data.LightViewProj = XMMatrixTranspose(shadowlightMatrices);
//	lcb.data.InvLightViewProj = XMMatrixInverse(nullptr, shadowlightMatrices);
//	lcb.data.Bias = 0.01f;
//	lcb.data.MapSize = gfx.depthMapResolution;
//	lcb.data.NumNodes = (UINT)Flat.size();
//	lcb.data.NumTris = (UINT)triangles.size();
//	lcb.data.LightDir = gfx.direction;
//	float clear[4] = {1.0,0.0,0.0,0.0};
//	
//	
//	lcb.ApplyChanges();
//	gfx.deviceContext->ClearUnorderedAccessViewFloat(gfx.shadowUAV.Get(), clear);
//
//	gfx.deviceContext->CSSetUnorderedAccessViews(0, 1, gfx.shadowUAV.GetAddressOf(), nullptr);
//	gfx.deviceContext->CSSetConstantBuffers(0, 1, lcb.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(0, 1, nodeSRV.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(1, 1, TrianglesSRV.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(2, 1, baseRootSRV.GetAddressOf());
//	gfx.deviceContext->CSSetShaderResources(3, 1, gfx.positionSRV.GetAddressOf());
//
//
//	m_BaseCB.data.NumBases = UINT(baseRoots.size());
//	m_BaseCB.ApplyChanges();
//	gfx.deviceContext->CSSetConstantBuffers(1, 1, m_BaseCB.GetAddressOf());
//
//	UINT gx = ((windowWidth/gfx.DownSampleMultiplier) + 15) / 16, gy = ((windowHeight/gfx.DownSampleMultiplier) + 15) / 16;
//	gfx.deviceContext->Dispatch(gx, gy, 1);
//	HRESULT rr = gfx.device->GetDeviceRemovedReason();
//	if (FAILED(rr))
//		ErrorLogger::Log(rr, "Graphics error");
//	ID3D11UnorderedAccessView* nullUAV = nullptr;
//	gfx.deviceContext->CSSetUnorderedAccessViews(0, 1, &nullUAV, nullptr);
//	ID3D11ShaderResourceView* nullsrv = nullptr;
//	gfx.deviceContext->CSSetShaderResources(0, 1, &nullsrv);
//	gfx.deviceContext->CSSetShaderResources(1, 1, &nullsrv);
//	gfx.deviceContext->CSSetShaderResources(2, 1, &nullsrv);
//	gfx.deviceContext->CSSetShaderResources(3, 1, &nullsrv);
//}

void Application::RenderFrame()
{
	
	if (RenderIrradianceandHDRI)
	{
		m_HDRIPass.Draw(&gfx);
		m_IrradianceConvolution.Draw(&gfx, m_HDRIPass.GetSRVRenderTarget()[0]);

		m_Prefiltering.Draw(& gfx, &m_HDRIPass.GetSRVRenderTarget()[0]);
		m_BRDFPass.Draw(&gfx);
		RenderIrradianceandHDRI = false;

	}

	m_GBuffer.Draw(&gfx, *m_Scene);

	//Square
	m_ShadowPass.Draw(&gfx, *m_Scene);
	//BackgroundCubeMap();
	LightingSRVData data;
	data.IrradianceSRV = m_IrradianceConvolution.GetSRVRenderTarget()[0];
	data.PrefilteringSRV = m_Prefiltering.GetSRVRenderTarget()[0];
	data.BRDFSRV = m_BRDFPass.GetSRVRenderTarget()[0];
	data.HDRIFrameBufferSRV = m_HDRIPass.GetSRVRenderTarget()[0];
	data.DirShadowMapSRV = m_ShadowPass.GetDirSRV();
	data.CascadeShadowMapSRV[0] = m_ShadowPass.GetCascades(0);
	data.CascadeShadowMapSRV[1] = m_ShadowPass.GetCascades(1);
	data.CascadeShadowMapSRV[2] = m_ShadowPass.GetCascades(2);
	data.CascadeShadowMapSRV[3] = m_ShadowPass.GetCascades(3);
	m_LightingPass.SetLightMatrixBuffers(m_ShadowPass);
	m_LightingPass.Draw(&gfx, m_GBuffer.GetGBufferSRV(), data);
	//if	(drawsurfeldebug)
	//{
	//	SpawnSurfels();
	//	DrawSurfels();
	//}
	////DrawShadowMaps();
	//DrawDebugCascade();

	//ForwardRender();
	// Start the Dear ImGui frame
	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	OnImguiRender();
	m_ShadowPass.ImGuiPass();
	m_SceneHierarchyPanel->OnImGuiRender();

	//Assemble Together Draw Data
	OnImguiRenderViewport();
	ImGui::Render();
	//Render Draw Data
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	auto io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	//m_lightparams.ApplyChanges();

	gfx.Present();
}

void Application::OnImguiRender()
{

	// Note: Switch this to true to enable dockspace
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = true;
	bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// DockSpace
	ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	style.WindowMinSize.x = minWinSizeX;

	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
				//OpenScene();

			ImGui::Separator();

			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
			{
				

			}

			if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				//SaveScene();

			if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				//SaveSceneAs();

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				//m_IsRunning = false;

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();
}

void Application::OnImguiRenderViewport()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
	ImGui::Begin("Viewport");
	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	XMFLOAT2 m_ViewportBounds[2];
	m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

	m_ViewportFocused = ImGui::IsWindowFocused();
	m_ViewportHovered = ImGui::IsWindowHovered();

	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	XMFLOAT2 m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };


	ImGui::Image((ImTextureID)m_LightingPass.GetSRVRenderTarget()[0], ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 1, 0 }, ImVec2{ 0, 1 });
	ImGui::PopStyleVar();

	ImGui::End();
}

//void Application::ForwardRender()
//{
//	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
//	gfx.ClearView(bgcolor);
//	gfx.SetInputLayout(m_vertexShader.GetInputLayout());
//	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	gfx.SetRasterizerState();
//	gfx.SetDepthStencilState();
//	gfx.SetBlendState();
//	gfx.SetSamplers();
//	gfx.SetVSShader(m_vertexShader.GetShader());
//	gfx.SetPSShader(m_pixelShader.GetShader());
//	gfx.SetPSConstantBuffers(0, 1, lightConstantBuffer.GetAddressOf());
//	gfx.SetPSConstantBuffers(1, 1, CameraInfoConstantBuffer.GetAddressOf());
//	this->lightConstantBuffer.data.dynamicLightColor = light.lightColor;
//	this->lightConstantBuffer.data.dynamicLightStrength = 1000.0f;
//	this->lightConstantBuffer.data.dynamicLightPosition = XMFLOAT3(0.0, 1.5, 0.0);
//	this->lightConstantBuffer.ApplyChanges();
//
//	RVec3 pos = gfx.physicsController.GetPosition(gameObject.GetID());
//	gameObject.SetPosition(pos.GetX(), pos.GetY(), pos.GetZ());
//	if (playercam)
//	{
//		{
//			//this->gameObject.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
//			floor.Draw(PlayerCamera.GetViewMatrix() * PlayerCamera.GetProjectionMatrix());
//		}
//	}
//	else
//	{
//		{
//			//this->gameObject.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
//			floor.Draw(gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix());
//		}
//
//	}
//}


bool Application::playercam = false;

