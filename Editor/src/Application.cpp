#include "Gamepch.h"
#include "Application.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"

#include <format>
#include "InputElements.h"
#include "src/Utils.h"
#include "Scripting/SciptMouse.h"
#include "Scripting/ScriptController.h"


void Application::Initialize(HINSTANCE hInstance, std::string window_title, std::string window_class, int width, int height)
{
	Engine::Project::SetEditorRoot("Editor");
	Engine::Project::SetProjectRoot("Game");
	EngineInit::Initialize(hInstance, window_title, window_class, width, height);
	windowWidth = width;
	windowHeight = height;
}

void Application::OnCreate()
{
	InitializeShaders();

	////Initialize Constant Buffer(s)
	//HRESULT hr = this->constantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	//COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	
	COM_ERROR_IF_FAILED(this->floorConstantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext()), "Failed to initialize constant buffer.");
	COM_ERROR_IF_FAILED(this->AnimatedConstantBuffer.Initialize(gfx.GetDevice(), gfx.GetDeviceContext()), "Failed to initialize constant buffer.");
	m_Scene = nullptr;
	auto newscene = std::make_unique<Scene>();
	m_Scene = std::move(newscene);


	gfx.camera.SetPosition(0.0,3.0,0.0);
	auto newDebugRenderer = std::make_unique<Engine::DebugRenderer>();
	m_DebugRenderer = std::move(newDebugRenderer);

	m_DebugRenderer->Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	Engine::DebugRenderer::Set(m_DebugRenderer.get());
	m_BRDFPass.Initialize(gfx.device.Get());
	m_HDRIPass.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_GBuffer.Initialize(gfx.GetDevice());
	m_IrradianceConvolution.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_LightingPass.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_Prefiltering.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	m_ShadowPass.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());

	std::filesystem::path fullPath =
		Engine::Project::ResolveAssetPath("Scene/TestScene1.DOE");
	m_Scene->LoadScene(fullPath.string());

	m_Scene->InitializeRuntimeResources(gfx.GetDevice(), gfx.GetDeviceContext(), AnimatedConstantBuffer, floorConstantBuffer);
	ProbeVolumeDesc sponzaProbes = {};

	AABB SceneAABB = m_Scene->GetSceneAABB();

	sponzaProbes.Min = SceneAABB.Minf();
	sponzaProbes.Max = SceneAABB.Maxf();
	sponzaProbes.Spacing = 20.f;
	sponzaProbes.CaptureRadius = 40.0f;

	m_Probes = CreateProbeGrid(sponzaProbes);
	
	m_ProbeCubemap.Initialize(gfx.GetDevice(), gfx.GetDeviceContext(), 64, m_Probes);
	
	
	m_SceneHierarchyPanel = std::make_unique<Editor::SceneHierarchyPanel>(m_Scene.get(), gfx.device.Get(), gfx.deviceContext.Get(), &floorConstantBuffer, &AnimatedConstantBuffer);




}

void Application::InitializeShaders()
{


}
void Application::OnUpdate()
{

	Engine::ScriptInput::SetKeyboard(&this->keyboard);
	Engine::ScriptMouse::SetMouse(&mouse);
	Engine::ScriptController::SetController(&controller);
	while (this->ProcessMessages() == true && m_IsRunning)
	{	
		float dtMs = timer.GetMilisecondsElapsed();
		float dt = dtMs * 0.001f;
		timer.Restart();
		this->Update();
		m_Scene->UpdateScene(dt);
		Engine::DebugRenderer::Get()->Update(dt);
		this->RenderFrame();
		if (m_Scene->IsPlaying())
		{
			controller.Update();
			m_Scene->UpdateWorldTransforms();
			m_Scene->UpdateRuntimeCamera(gfx);
			m_Scene->UpdateAgents(dt);  
			m_Scene->UpdatePhysicsTransforms();
			m_Scene->PlayUpdate(dt);
			m_Scene->FlushDestroyedEntities();

			mouse.EndFrame();
		}


		const float cameraSpeed = 5.0f;
		
		if (!m_Scene->IsPlaying())
		{
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
				this->gfx.camera.AdjustPosition(0.0f, cameraSpeed * dt, 0.0f);
			}
			if (keyboard.KeyIsPressed(VK_CONTROL))
			{
				this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed * dt, 0.0f);
			}
		}
		/*if (keyboard.KeyIsPressed('Z'))
		{
			this->gfx.camera.AdjustPosition(0.0f, -cameraSpeed, 0.0f);
		}*/
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
	//Engine::DebugRenderer::Get()->BeginFrame();
	if (RenderIrradianceandHDRI)
	{
		m_HDRIPass.Draw(&gfx);
		m_IrradianceConvolution.Draw(&gfx, m_HDRIPass.GetSRVRenderTarget()[0]);

		m_Prefiltering.Draw(& gfx, &m_HDRIPass.GetSRVRenderTarget()[0]);
		m_BRDFPass.Draw(&gfx);
		RenderIrradianceandHDRI = false;

	}
	if (generateProbes)
	{
		m_ProbeCubemap.Draw(&gfx, m_Probes, *m_Scene);
		generateProbes = false;
	}
	m_GBuffer.Draw(&gfx, *m_Scene);
	//Square
	m_ShadowPass.Draw(&gfx, *m_Scene);
	//BackgroundCubeMap();
	m_ProbeCubemap.LightSurfels(&gfx,m_ShadowPass.GetDirSRV(), m_ShadowPass.GetDirShadowMatrix(), m_ShadowPass.GetLightDir(), m_LightingPass.GetLightColor());
	m_ProbeCubemap.NormalizeAndAccumilate(&gfx);
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
	data.ProbesSRV = m_ProbeCubemap.GetProbesSRV();
	m_LightingPass.SetLightMatrixBuffers(m_ShadowPass);
	m_LightingPass.Draw(&gfx, m_GBuffer.GetGBufferSRV(), data, m_Probes.size());

	if (m_DrawDebug)
	{
		m_Scene->DrawNavMesh();

	}


	Engine::DebugRenderer::Get()->Flush(gfx.GetDeviceContext(), (gfx.camera.GetViewMatrix() * gfx.camera.GetProjectionMatrix()));

	// Start the Dear ImGui frame
	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	OnImguiRender();
	m_ShadowPass.ImGuiPass();
	m_SceneHierarchyPanel->OnImGuiRender();
	m_LightingPass.ImGuiPass();
	ImGui::Begin("NavMesh");
	if (ImGui::Button("Generate NavMesh"))
	{
		m_Scene->CreateNavMesh();
	}
	ImGui::End();
	
	ImGui::Begin("Settings");
	ImGui::Checkbox("Draw Debug", &m_DrawDebug);
	if (ImGui::Button("GenerateProbes"))
	{
		generateProbes = true;
	}

	if (!m_Scene->IsPlaying())
	{
		if (ImGui::Button("Play"))
		{
			m_EditorSaveScene = m_Scene->Copy();
			gfx.Editorcamera = gfx.camera.Clone();
			m_Scene->Play();

		}

	}
	else
	{
		if (ImGui::Button("Stop"))
		{
			m_Scene->Stop();

			gfx.camera = gfx.Editorcamera;

			m_Scene = std::move(m_EditorSaveScene);

			m_Scene->InitializeRuntimeResources(gfx.GetDevice(), gfx.GetDeviceContext(), AnimatedConstantBuffer, floorConstantBuffer);
			m_EditorSaveScene = nullptr;
			m_SceneHierarchyPanel->SetContext(m_Scene.get());
		}
	}
	
	if (ImGui::Button("Reload Scripts"))
	{
		m_Scene->ReloadScript();
	}
	
	
	char pathBuffer[512] = {};
	strncpy_s(pathBuffer, sizeof(pathBuffer), m_GameProjectPath.c_str(), _TRUNCATE);

	if (ImGui::InputText("GameProjectDir", pathBuffer, sizeof(pathBuffer)))
	{
		m_GameProjectPath = std::string(pathBuffer);

		std::filesystem::path enteredPath =
			std::filesystem::absolute(std::filesystem::path(m_GameProjectPath));

		std::filesystem::path projectRoot;

		if (enteredPath.filename() == "Assets")
			projectRoot = enteredPath.parent_path();
		else
			projectRoot = enteredPath;

		std::filesystem::path assetsPath = projectRoot / "Assets";

		if (std::filesystem::exists(assetsPath))
		{
			Engine::Project::SetProjectRoot(projectRoot);
		}
		else
		{
			ErrorLogger::Log("Invalid game project path. Assets folder not found.\n");
		}
		
	}
	
	ImGui::End();


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
				OpenScene();

			ImGui::Separator();

			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
			{
				m_Scene->ClearScene();

			}

			if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				SaveScene();

			if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				SaveSceneAs();

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				m_IsRunning = false;

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

	ImGui::End();
}
void Application::SaveScene()
{
	if (!m_EditorScenePath.empty())
		m_Scene->SerializeScene(m_EditorScenePath);
	else
		SaveSceneAs();
}

void Application::SaveSceneAs()
{
	std::string filepath = Utils::SaveFile("DOE Scene (*.DOE)\0*.DOE\0", RenderWindow::GetHWND());
	if (!filepath.empty())
	{
		m_Scene->SerializeScene(filepath);
		m_EditorScenePath = filepath;
	}

}
void Application::OpenScene()
{
	std::string filepath = Utils::OpenFile("DOE Scene (*.DOE)\0*.DOE\0", RenderWindow::GetHWND());
	if (!filepath.empty())
	{
		m_Scene->LoadScene(filepath);
		m_Scene->InitializeRuntimeResources(gfx.GetDevice(), gfx.GetDeviceContext(), AnimatedConstantBuffer, floorConstantBuffer);
	}
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

