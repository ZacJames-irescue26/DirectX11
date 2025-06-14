#include "Gamepch.h"
#include "Application.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <format>


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
	
	hr = m_PrefilteringParams.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");
	
	hr = m_LightSpace.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize constant buffer.");

	hr = m_ObjectModel.Initialize(gfx.GetDevice(), gfx.GetDeviceContext());
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
	m_CastLight.data.light.position = XMFLOAT3(0.0,5.0,0.0);
	m_CastLight.data.light.intensity = XMFLOAT3(1.0, 1.0, 1.0);
	m_CastLight.data.light.direction = XMFLOAT3(0.0, -1.0, 0.0);
	m_CastLight.data.light.cutOff = 0.9;
	m_CastLight.ApplyChanges();

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
	//9s without threading
	//2.9s after threading

	/*if (!MiscItems.Initialize(
		"Assets/MiscItems.gltf", gfx.GetDevice(),
		gfx.GetDeviceContext(), this->floorConstantBuffer))
	{
		return;
	}*/
	helmet.SetScale({1,1,1});
	helmet.SetPosition(XMFLOAT3{0.0,10.0,0.0});
	floor.SetPosition(XMVECTOR{-.1f,-0.1,-0.1});
	floor.SetScale({0.01,0.01,0.01});
	/*MiscItems.SetPosition(XMVECTOR{0.0,-2.0,0.0});
	MiscItems.SetScale({10,10,10});*/
	gfx.physicsController.Optimize();

	PlayerCamera.SetPosition(0.0,0.0,-4.0);
	PlayerCamera.SetProjectionValues(90, static_cast<float>(gfx.windowWidth) / static_cast<float>(gfx.windowHeight), 0.1f, 1000.0f);
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
	m_lightparams.data.LightColor = DirectX::XMFLOAT3(10.0, 10.0, 10.0);
	m_lightparams.data.LightDirection = DirectX::XMFLOAT3(0.0, -4.8, -1.0);
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

	AABB testbox = AABB(XMFLOAT3{-15.0,-5.0,-15.0},{15.0,15.0,15.0});
	octree = new Octree(&testbox,5);
	std::vector<GameObject> objects;
	objects.push_back(floor);
	gen = new SurfelGenerator(gfx.GetDevice(), gfx.GetDeviceContext(), octree, objects );

	std::vector<SurfelVB> svb;
	for (const auto& surf : gen->GeneratedSurfels)
	{
		SurfelVB vb;
		vb.pos = surf->position;
		vb.norm = surf->normal;
		vb.color = surf->albedo;
		vb.radius = surf->radius;
		svb.push_back(vb);
	}
	SurfelVertexBuffer.Initialize(gfx.device.Get(), svb.data(), svb.size());

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
	D3D11_INPUT_ELEMENT_DESC ModelPos[] = 
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0, offsetof(Vertex, pos), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0}
	};

	D3D11_INPUT_ELEMENT_DESC Surfelvb[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0, offsetof(SurfelVB, pos), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0},
		{ "NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0, offsetof(SurfelVB, norm), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0, offsetof(SurfelVB, color), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0 },
		{ "RADIUS",0,DXGI_FORMAT_R32_FLOAT,0, offsetof(SurfelVB, radius), D3D11_INPUT_CLASSIFICATION::D3D11_INPUT_PER_VERTEX_DATA,0 }
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
	if (!m_Prefiltering_PS.Initialize(gfx.device, L"CompiledShaders/Prefiltering_p.cso"))
	{
		return;
	}
	if (!m_BRDF_VS.Initialize(gfx.device, L"CompiledShaders/BRDF_v.cso", FullScreenRectlayout, ARRAYSIZE(FullScreenRectlayout)))
	{
		return;
	}	
	if (!m_BRDF_PS.Initialize(gfx.device, L"CompiledShaders/BRDF_p.cso"))
	{
		return;
	}
	if (!m_ShadowDepth_VS.Initialize(gfx.device, L"CompiledShaders/ShadowMapDepth_v.cso", ModelPos, ARRAYSIZE(ModelPos)))
	{
		return;
	}
	if (!m_ShadowDepth_GS.Initialize(gfx.device, L"CompiledShaders/ShadowMapDepth_g.cso"))
	{
		return;
	}
	if (!m_DebugCascade_VS.Initialize(gfx.device, L"CompiledShaders/DebugCascade_v.cso", ModelPos, ARRAYSIZE(ModelPos)))
	{
		return;
	}
	if (!m_DebugCascade_PS.Initialize(gfx.device, L"CompiledShaders/DebugCascade_p.cso"))
	{
		return;
	}
	if (!m_DebugDrawShadowMap_PS.Initialize(gfx.device, L"CompiledShaders/DrawShadowMap_p.cso"))
	{
		return;
	}

	if (!m_SurfelDebug_VS.Initialize(gfx.device, L"CompiledShaders/SurfelDebug_v.cso", Surfelvb, ARRAYSIZE(Surfelvb)))
	{
		return;
	}
	if (!m_SurfelDebug_PS.Initialize(gfx.device, L"CompiledShaders/SurfelDebug_p.cso"))
	{
		return;
	}
	if (!m_SureflDebug_GS.Initialize(gfx.device, L"CompiledShaders/SurfelDebug_g.cso"))
	{
		return;
	}
	if (!m_Shadow_CS.Initialize(gfx.device, L"CompiledShaders/RealtimeShadows_c.cso"))
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
	gfx.deviceContext->OMSetDepthStencilState(
		gfx.depthStencilState.Get(),   // from InitializeDirectX
		/*stencilRef=*/0);
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
			floor.Draw(PlayerCamera.GetProjectionMatrix() * PlayerCamera.GetViewMatrix());
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
	ID3D11RenderTargetView* nullview[] = {nullptr, nullptr, nullptr, nullptr};
	gfx.GetDeviceContext()->OMSetRenderTargets(4, nullview, nullptr);
	gfx.deviceContext->PSSetShader(nullptr, nullptr, 0);
	gfx.deviceContext->VSSetShader(nullptr, nullptr, 0);
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

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);

	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(),0);
	gfx.SetSamplers();
	gfx.GetDeviceContext()->PSSetSamplers(1, 1, gfx.HDRIsamplerState.GetAddressOf());
	gfx.GetDeviceContext()->PSSetSamplers(2, 1, gfx.shadowSampler.GetAddressOf());
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
		gfx.HDRIFramebufferSRV.Get(),
		gfx.PrefilteringSRV.Get(),
		gfx.BRDFSRV.Get(),
		gfx.shadowSRVs[0].Get(),
		gfx.shadowSRVs[1].Get(),
		gfx.shadowSRVs[2].Get(),
		gfx.shadowSRVs[3].Get(),
		gfx.DepthBuffer.Get(),
	};

	gfx.GetDeviceContext()->PSSetShaderResources(0, shaderresources.size(), shaderresources.data());
	
	std::vector<XMMATRIX> LightMatrcies = gfx.m_CascadeLightVP;

	m_lightparams.data.LightSpaceMatrices0 = LightMatrcies[0];
	m_lightparams.data.LightSpaceMatrices1 = LightMatrcies[1];
	m_lightparams.data.LightSpaceMatrices2 = LightMatrcies[2];
	m_lightparams.data.LightSpaceMatrices3 = LightMatrcies[3];
	m_lightparams.data.cascadePlaneDistances.x = gfx.shadowCascadeLevels[0];
	m_lightparams.data.cascadePlaneDistances.y = gfx.shadowCascadeLevels[1];
	m_lightparams.data.cascadePlaneDistances.z = gfx.shadowCascadeLevels[2];
	m_lightparams.data.cascadePlaneDistances.w = gfx.shadowCascadeLevels[3];

	m_lightparams.data.farplane = 1000;
	m_lightparams.ApplyChanges();

	gfx.SetPSConstantBuffers(0,1, m_lightparams.GetAddressOf()); 
	CameraInfoConstantBuffer.data.CameraPosition = gfx.camera.GetPositionFloat3();
	CameraInfoConstantBuffer.data.InvProj = XMMatrixTranspose(XMMatrixInverse(nullptr, gfx.camera.GetProjectionMatrix()));
	CameraInfoConstantBuffer.data.InvView = XMMatrixTranspose(XMMatrixInverse(nullptr, gfx.camera.GetViewMatrix()));
	CameraInfoConstantBuffer.data.View = XMMatrixTranspose(gfx.camera.GetViewMatrix());

	CameraInfoConstantBuffer.ApplyChanges();
	gfx.SetPSConstantBuffers(1,1, CameraInfoConstantBuffer.GetAddressOf());
	gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());
	gfx.SetPSConstantBuffers(2, 1, m_CastLight.GetAddressOf());
	m_CastLight.ApplyChanges();


	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);
	
	gfx.GetDeviceContext()->DrawIndexed(6,0,0);

	ID3D11ShaderResourceView* nullSRVs[13] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,nullptr, nullptr, nullptr, nullptr };
	gfx.GetDeviceContext()->PSSetShaderResources(0, 12, nullSRVs);
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

	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.01f, 100.0f);

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
	gfx.GetDeviceContext()->GenerateMips(gfx.HDRIFramebufferSRV.Get());
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

	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	const UINT baseSize = 128;
	const UINT mipLevels = static_cast<UINT>(std::floor(std::log2(baseSize))) + 1;
	UINT baseResolution = 128;
	gfx.deviceContext->PSSetSamplers(0, 1, gfx.PrefilteredsamplerState.GetAddressOf());
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.HDRIdepthStencilStateDisabled.Get(), 0);

	gfx.SetInputLayout(this->m_EquiToHDRI_VS.GetInputLayout());
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->VSSetShader(m_EquiToHDRI_VS.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_Prefiltering_PS.GetShader(), NULL, 0);

	for (UINT mip = 0; mip < mipLevels; ++mip)
	{
		UINT mipWidth = static_cast<UINT>(baseResolution * std::pow(0.5f, mip));
		UINT mipHeight = static_cast<UINT>(baseResolution * std::pow(0.5f, mip));

		// Resize viewport
		D3D11_VIEWPORT previewport = {};
		previewport.TopLeftX = 0;
		previewport.TopLeftY = 0;
		previewport.Width = mipWidth;
		previewport.Height = mipHeight;
		previewport.MinDepth = 0.0f;
		previewport.MaxDepth = 1.0f;
		gfx.GetDeviceContext()->RSSetViewports(1, &previewport);
		gfx.GetDeviceContext()->PSSetShaderResources(0,1, gfx.HDRIFramebufferSRV.GetAddressOf());
		gfx.GetDeviceContext()->PSSetConstantBuffers(0, 1, m_PrefilteringParams.GetAddressOf());
		float roughness = (float)mip / (float)(mipLevels - 1);
		m_PrefilteringParams.data.roughness = roughness;
		m_PrefilteringParams.ApplyChanges();




		for (UINT face = 0; face < 6; ++face)
		{
			XMMATRIX views[6] = {
				XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(1, 0, 0, 0), XMVectorSet(0, 1, 0, 0)), // +X
				XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(-1, 0, 0, 0), XMVectorSet(0, 1, 0, 0)), // -X
				XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 1, 0, 0), XMVectorSet(0, 0, -1, 0)), // +Y
				XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, -1, 0, 0), XMVectorSet(0, 0, 1, 0)), // -Y
				XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 0, 1, 0), XMVectorSet(0, 1, 0, 0)),  // +Z
				XMMatrixLookAtLH(XMVectorZero(), XMVectorSet(0, 0, -1, 0), XMVectorSet(0, 1, 0, 0)), // -Z
			};

			XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV2, 1.0f, 0.1f, 10.0f);
			// Set view matrix for that face
			HDRIViewProj.data.View = XMMatrixTranspose(views[face]);
			HDRIViewProj.data.Projection = XMMatrixTranspose(proj);
			HDRIViewProj.ApplyChanges();
			gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, HDRIViewProj.GetAddressOf());

			// Bind the correct mip level and face


			gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.PrefilteringRTVs[mip][face].GetAddressOf(), nullptr);
			gfx.GetDeviceContext()->ClearRenderTargetView(gfx.PrefilteringRTVs[mip][face].Get(), bgcolor);


			gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
			gfx.GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

			gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
		}
	}
	ID3D11RenderTargetView* nullRTVs[1] = { nullptr };
	gfx.GetDeviceContext()->OMSetRenderTargets(1, nullRTVs, nullptr);
	ID3D11SamplerState* nullSampler[1] = { nullptr };
	gfx.GetDeviceContext()->PSSetSamplers(0, 1, nullSampler);
	ID3D11Buffer* nullBuffer = nullptr;
	gfx.SetVSConstantBuffers(0, 1, &nullBuffer);
}
void Application::BRDF()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	// Resize viewport
	D3D11_VIEWPORT BRDFviewport = {};
	BRDFviewport.TopLeftX = 0;
	BRDFviewport.TopLeftY = 0;
	BRDFviewport.Width = 512;
	BRDFviewport.Height = 512;
	BRDFviewport.MinDepth = 0.0f;
	BRDFviewport.MaxDepth = 1.0f;
	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.BRDFRTVs.GetAddressOf(), nullptr);

	gfx.GetDeviceContext()->RSSetViewports(1, &BRDFviewport);
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(), 0);
	gfx.SetInputLayout(this->m_BRDF_VS.GetInputLayout());

	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_BRDF_VS.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_BRDF_PS.GetShader(), NULL, 0);

	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	gfx.GetDeviceContext()->DrawIndexed(6, 0, 0);



}
void Application::BackgroundCubeMap()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(),nullptr);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.renderTargetView.Get(), bgcolor);

	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilSkyboxState.Get(), 0);
	gfx.GetDeviceContext()->PSSetSamplers(0, 1, gfx.HDRIsamplerState.GetAddressOf());

	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_BackgroundCubemap_VS.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_BackgroundCubemap_PS.GetShader(), NULL, 0);


	gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	gfx.GetDeviceContext()->PSSetShaderResources(0, 1, gfx.HDRIFramebufferSRV.GetAddressOf());
	
	gfx.SetVSConstantBuffers(0,1,HDRIViewProj.GetAddressOf());

	HDRIViewProj.data.Projection = gfx.camera.GetProjectionMatrix();
	XMMATRIX view = gfx.camera.GetViewMatrix();
	view.r[3] = XMVectorSet(0, 0, 0, 1); // zero translation
	HDRIViewProj.data.View = XMMatrixTranspose(view);
	HDRIViewProj.ApplyChanges();


	gfx.SetInputLayout(this->m_BackgroundCubemap_VS.GetInputLayout());

	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_HdriVertex.GetAddressOf(), this->m_HdriVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_HdriIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
}

void Application::ShadowDepthPass()
{
	gfx.CalcCascadeOrthoProjs();
	const std::vector<XMMATRIX> lightMatrices = gfx.m_CascadeLightVP;
	for (UINT i = 0; i < gfx.NUM_CASCADES; i++)
	{
		D3D11_VIEWPORT shadowViewport = {};
		shadowViewport.TopLeftX = 0;
		shadowViewport.TopLeftY = 0;
		shadowViewport.Width = gfx.depthMapResolution;
		shadowViewport.Height = gfx.depthMapResolution;
		shadowViewport.MinDepth = 0.0f;
		shadowViewport.MaxDepth = 1.0f;
		// 1. Set viewport (matching your shadow map resolution)

		// 2. Set depth-only render target for this cascade
		gfx.GetDeviceContext()->OMSetRenderTargets(0, nullptr, gfx.shadowDSVs[i].Get());

		gfx.SetRasterizerState();
		gfx.SetBlendState();
		gfx.GetDeviceContext()->RSSetViewports(1, &shadowViewport);
		// 3. Clear depth buffer
		gfx.GetDeviceContext()->ClearDepthStencilView(gfx.shadowDSVs[i].Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
		gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.shadowDepthStencilState.Get(), 0);

		// 4. Set shaders
		gfx.GetDeviceContext()->VSSetShader(m_ShadowDepth_VS.GetShader(), nullptr, 0);
		gfx.GetDeviceContext()->GSSetShader(nullptr, nullptr, 0); // Optional: geometry shader to set gl_Layer
		gfx.SetInputLayout(this->m_ShadowDepth_VS.GetInputLayout());
		gfx.GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);           // No pixel shader for depth-only
		//gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_LightSpace.GetAddressOf());
		gfx.GetDeviceContext()->VSSetConstantBuffers(1,1, m_LightSpace.GetAddressOf());
		// 5. Set view/projection matrix for this cascade
		
		m_LightSpace.data.LightSpace = lightMatrices[i];
	
		m_LightSpace.ApplyChanges();
		// 6. Draw scene
		gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ObjectModel.GetAddressOf());
		
		
		m_ObjectModel.data.Model = XMMatrixTranspose(helmet.worldMatrix);
		gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		m_ObjectModel.ApplyChanges();
		this->helmet.DrawWithOutCBuffer();
		m_ObjectModel.data.Model = XMMatrixTranspose(floor.worldMatrix);
		m_ObjectModel.ApplyChanges();
		floor.DrawWithOutCBuffer();

	}

	gfx.GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);
}
void Application::DrawDebugCascade()
{
	DWORD cascadeindices[] = {
		0, 2, 3,
		0, 3, 1,
		4, 6, 2,
		4, 2, 0,
		5, 7, 6,
		5, 6, 4,
		1, 3, 7,
		1, 7, 5,
		6, 7, 3,
		6, 3, 2,
		1, 5, 4,
		0, 1, 4
	};
	m_DebugCascade.resize(8);
	for (int i = 0; i < gfx.NUM_CASCADES; i++)
	{
		const auto corners = gfx.getFrustumCornersWorldSpace(gfx.m_CascadeLightVP[i]);
		std::vector<CubeWPos> vec3s;
		vec3s.resize(corners.size());
		for (int i = 0; i < corners.size(); i++)
		{
			XMFLOAT3 pos;
			XMStoreFloat3(&pos, corners[i]);
			vec3s[i].pos = pos;
		}

		m_DebugCascade[i].Initialize(gfx.device.Get(), vec3s.data(), vec3s.size());

	}
	m_CascadeIndex.Initialize(gfx.device.Get(), cascadeindices, 36);
	float blendFactor[4] = { 0, 0, 0, 0 }; // usually ignored unless BlendFactor used
	UINT sampleMask = 0xffffffff;
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	gfx.SetInputLayout(m_DebugCascade_VS.GetInputLayout());
	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.SetDepthStencilState();
	gfx.GetDeviceContext()->OMSetBlendState(gfx.transparentBlendState.Get(),blendFactor, sampleMask);
	gfx.SetSamplers();
	gfx.SetVSShader(m_DebugCascade_VS.GetShader());
	gfx.SetPSShader(m_DebugCascade_PS.GetShader());

	gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ViewProj.GetAddressOf());

	m_ViewProj.data.View = XMMatrixTranspose(gfx.camera.GetViewMatrix());
	m_ViewProj.data.Projection = XMMatrixTranspose(gfx.camera.GetProjectionMatrix());
	m_ViewProj.ApplyChanges();
	gfx.GetDeviceContext()->PSSetConstantBuffers(0,1, m_DebugColors.GetAddressOf());
	gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);


	for (int i = 0; i < gfx.NUM_CASCADES; i++)
	{
		m_DebugColors.data.index = i;
		m_DebugColors.ApplyChanges();
		gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_DebugCascade[i].GetAddressOf(), this->m_DebugCascade[i].StridePtr(), &offset);
		gfx.GetDeviceContext()->IASetIndexBuffer(m_CascadeIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

		gfx.GetDeviceContext()->DrawIndexed(36, 0, 0);
	}

}
void Application::DrawShadowMaps()
{
	float bgcolor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
	gfx.GetDeviceContext()->ClearRenderTargetView(gfx.renderTargetView.Get(), clearColor);

	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetRasterizerState();
	gfx.SetBlendState();
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(), 0);
	gfx.SetSamplers();
	gfx.SetInputLayout(this->m_DeferredvertexShader.GetInputLayout());

	gfx.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	gfx.GetDeviceContext()->VSSetShader(m_DeferredvertexShader.GetShader(), NULL, 0);
	gfx.GetDeviceContext()->PSSetShader(m_DebugDrawShadowMap_PS.GetShader(), NULL, 0);


	gfx.GetDeviceContext()->PSSetShaderResources(0, 1,gfx.DirectionalshadowSRVs.GetAddressOf());

	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->m_FullScreenVertex.GetAddressOf(), this->m_FullScreenVertex.StridePtr(), &offset);
	gfx.GetDeviceContext()->IASetIndexBuffer(m_FullScreenIndex.Get(), DXGI_FORMAT_R32_UINT, 0);

	gfx.GetDeviceContext()->DrawIndexed(6, 0, 0);
}
//void Application::DirectionalShadowMap()
//{
//
//	if (gfx.LightDir.y == 0.0)
//	{
//		return ;
//	}
//
//	 XMMATRIX ShadowProj = XMMatrixPerspectiveFovLH(
//		XMConvertToRadians(60.0f),1.77,0.1, farplane);
//	//XMMATRIX ShadowOrtho = XMMatrixOrthographicOffCenterLH(-30.0f, 30.00, -30.0f, 30.0f, 0.01, farplane);
//	float theta = M_PI * Sky.x;
//	float phi = 2 * M_PI * Sky.y;
//	direction = XMFLOAT3(sin(theta) * sin(phi), cos(theta), sin(theta) * cos(phi));
//	XMFLOAT3 up_vec = XMFLOAT3(0.0, 1.0, 0.0);
//	if (Sky.x < -0.1 || Sky.x > -0.9) up_vec = XMFLOAT3(0.0, 0.0, 1.0);
//	// Move light x units away from the target along lightDir
//	XMVECTOR target = XMLoadFloat3(&TargetVec);
//	
//	XMVECTOR lightDir = XMVector3Normalize(XMLoadFloat3(&direction));
//	XMVECTOR eye = XMVectorMultiplyAdd(lightDir, XMVectorReplicate(-shadowDirstance), target);
//	XMMATRIX lightView = XMMatrixLookAtLH(eye, target, XMLoadFloat3(&up_vec));
//	XMMATRIX lightProj = XMMatrixOrthographicOffCenterLH(-100, 100, -40, 40, 0.1f, farplane); // Better for directional light
//	lightMatrices = XMMatrixTranspose(lightView * lightProj);
//	//XMMATRIX lightView = XMMatrixLookAtLH(shadowDirstance * XMLoadFloat3(&direction), target, XMLoadFloat3(&up_vec));
////	lightMatrices = XMMatrixMultiply(lightView, ShadowProj);
//
//
//	D3D11_VIEWPORT shadowViewport = {};
//	shadowViewport.TopLeftX = 0;
//	shadowViewport.TopLeftY = 0;
//	shadowViewport.Width = gfx.depthMapResolution;
//	shadowViewport.Height = gfx.depthMapResolution;
//	shadowViewport.MinDepth = 0.0f;
//	shadowViewport.MaxDepth = 1.0f;
//	// 1. Set viewport (matching your shadow map resolution)
//	// 2. Set depth-only render target for this cascade
//	gfx.GetDeviceContext()->OMSetRenderTargets(0, nullptr, gfx.DirectionalshadowDSVs.Get());
//	gfx.SetRasterizerState();
//	gfx.SetBlendState();
//	gfx.GetDeviceContext()->RSSetViewports(1, &shadowViewport);
//	// 3. Clear depth buffer
//	gfx.GetDeviceContext()->ClearDepthStencilView(gfx.DirectionalshadowDSVs.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
//	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.shadowDepthStencilState.Get(), 0);
//	// 4. Set shaders
//	gfx.GetDeviceContext()->VSSetShader(m_ShadowDepth_VS.GetShader(), nullptr, 0);
//	gfx.SetInputLayout(this->m_ShadowDepth_VS.GetInputLayout());
//	gfx.GetDeviceContext()->PSSetShader(nullptr, nullptr, 0);           // No pixel shader for depth-only
//	//gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_LightSpace.GetAddressOf());
//	gfx.GetDeviceContext()->VSSetConstantBuffers(1, 1, m_LightSpace.GetAddressOf());
//	// 5. Set view/projection matrix for this cascade
//	m_LightSpace.data.LightSpace = lightMatrices;
//	m_LightSpace.ApplyChanges();
//	// 6. Draw scene
//	gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ObjectModel.GetAddressOf());
//	m_ObjectModel.data.Model = XMMatrixTranspose(helmet.worldMatrix);
//	gfx.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//	m_ObjectModel.ApplyChanges();
//	this->helmet.DrawWithOutCBuffer();
//	m_ObjectModel.data.Model = XMMatrixTranspose(floor.worldMatrix);
//	m_ObjectModel.ApplyChanges();
//	floor.DrawWithOutCBuffer();
//
//	
//}
void Application::DrawSurfels()
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	gfx.GetDeviceContext()->OMSetRenderTargets(1, gfx.renderTargetView.GetAddressOf(), nullptr);
	gfx.GetDeviceContext()->OMSetDepthStencilState(gfx.depthStencilStateDisabled.Get(), 0);
	gfx.GetDeviceContext()->RSSetViewports(1, &viewport);
	gfx.SetBlendState();
	gfx.SetDepthStencilState();
	gfx.SetInputLayout(m_SurfelDebug_VS.GetInputLayout());
	gfx.SetRasterizerState();
	gfx.SetTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	gfx.GetDeviceContext()->IASetVertexBuffers(0, 1, this->SurfelVertexBuffer.GetAddressOf(), this->SurfelVertexBuffer.StridePtr(), &offset);
	gfx.GetDeviceContext()->VSSetConstantBuffers(0, 1, m_ViewProj.GetAddressOf());
	gfx.GetDeviceContext()->GSSetConstantBuffers(0, 1, m_ViewProj.GetAddressOf());
	
	m_ViewProj.data.Projection = XMMatrixTranspose(gfx.camera.GetProjectionMatrix());
	m_ViewProj.data.View = XMMatrixTranspose(gfx.camera.GetViewMatrix());
	m_ViewProj.ApplyChanges();
	gfx.SetVSShader(m_SurfelDebug_VS.GetShader());
	gfx.SetPSShader(m_SurfelDebug_PS.GetShader());
	gfx.GetDeviceContext()->GSSetShader(m_SureflDebug_GS.GetShader(), nullptr, 0);

	gfx.GetDeviceContext()->Draw(gen->GeneratedSurfels.size(), 0);
	gfx.GetDeviceContext()->GSSetShader(nullptr, nullptr, 0);

}

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
		DrawHDRI();
		IrradianceConvolution();
		Prefiltering();
		BRDF();
		RenderIrradianceandHDRI = false;

	}
	BindGBufferPass();
	

	//Square
	ShadowDepthPass();
	BackgroundCubeMap();
	BindLightingPass();
	if (drawsurfeldebug)
	{
		DrawSurfels();
	}
	//DrawShadowMaps();
	DrawDebugCascade();

	//ForwardRender();
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//Create ImGui Test Window
	gfx.LightDir = m_lightparams.data.LightDirection;
	ImGui::Begin("Light Controls");
	m_lightparams.data.LightDirection = gfx.direction;
	if (ImGui::TreeNode("Dir Light"))
	{
		//ImGui::DragFloat3("Light direction", &m_lightparams.data.LightDirection.x, 0.1f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("direction", &gfx.direction.x, 0.1f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("Light color ", &m_lightparams.data.LightColor.x, 0.1f, -0.0f, 1000.0f);
		ImGui::DragFloat2("SkyDir", &gfx.Sky.x, 0.001, -1, 1);
		static XMFLOAT3 helpos = {0.0,10.0,0.0};
		ImGui::DragFloat3("helmet pos", &helpos.x, 0.1, -10, 1000);
		helmet.SetPosition(helpos);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Spot Light"))
	{
		ImGui::DragFloat3("Light Position", &m_CastLight.data.light.position.x, 0.1f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("Light direction", &m_CastLight.data.light.direction.x, 0.1f, -1000.0f, 1000.0f);
		ImGui::DragFloat3("Light Intensity", &m_CastLight.data.light.intensity.x, 0.1f, 1.0f, 1000.0f);
		ImGui::DragFloat("Light CutOff", &m_CastLight.data.light.cutOff, 0.1, 0.0, 100.0);
		m_CastLight.ApplyChanges();
		ImGui::TreePop();
	}
	ImGui::DragInt("Shadowmap index ", &shadowmapIndex, 1, 0, 3);
	ImGui::DragFloat("shadow distance ", &gfx.shadowDirstance, 1, 1, 10000);
	XMFLOAT4X4 lightmat;
	XMStoreFloat4x4(&lightmat, XMMatrixTranspose(gfx.camera.GetViewMatrix()));
	ImGui::Text(std::string(std::format("{}, {}, {}, {} \n {}, {}, {}, {} \n {}, {}, {}, {} \n {}, {}, {}, {}", 
	lightmat._11, lightmat._12,lightmat._13,lightmat._14,
	lightmat._21,lightmat._22,lightmat._23,lightmat._24,
		lightmat._31, lightmat._32, lightmat._33, lightmat._34,
		lightmat._41, lightmat._42, lightmat._43, lightmat._44)).c_str());
	
	
	ImGui::DragFloat("farplane ", &farplane, 1, 10, 10000);
	ImGui::Checkbox("Player camera", &playercam);
	ImGui::Checkbox("Draw Surfels", &drawsurfeldebug);
	ImGui::Checkbox("Raytrace Shadows", &ifRaytraceShadows);
	ImGui::Checkbox("Take Shot", &TakeShot);
	//ImGui::Image((ImTextureID)gfx.positionSRV.Get(), { 200,200 });
	//ImGui::Image((ImTextureID)gfx.DirectionalshadowSRVs.Get(), { 200,200 });
	
	ImGui::Image((ImTextureID)gfx.DepthBuffer.Get(), {200,200});
	ImGui::Image((ImTextureID)gfx.shadowSRVs[1].Get(), { 200,200 });
	ImGui::Image((ImTextureID)gfx.shadowSRVs[2].Get(), { 200,200 });
	ImGui::Image((ImTextureID)gfx.shadowSRVs[3].Get(), { 200,200 });
	ImGui::Image((ImTextureID)gfx.DepthBuffer.Get(), {25,25 });
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


bool Application::playercam = false;

