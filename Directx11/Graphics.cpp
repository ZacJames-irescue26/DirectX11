#include "pch.h"
#include "Graphics.h"
#include "dxerr.h"
#include "GraphicsThrowMacros.h"



Graphics::Graphics(HWND hwnd, int width, int height)
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = width;
	sd.BufferDesc.Height = height;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// for checking results of d3d functions
	HRESULT hr;

	// create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_NOINFO( D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			nullptr,
			swapCreateFlags,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&sd,
			&pSwap,
			&pDevice,
			nullptr,
			&pContext
			)
	);
	 Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer = nullptr;
	GFX_THROW_NOINFO(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), (&pBackBuffer)));

	GFX_THROW_NOINFO(pDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &pTarget));
	
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState>pDSState;
	GFX_THROW_NOINFO(pDevice->CreateDepthStencilState(&dsDesc, &pDSState));

	pContext->OMSetDepthStencilState(pDSState.Get(), 1u);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 800u;
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT; 
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT; 
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	GFX_THROW_NOINFO(pDevice->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_NOINFO(pDevice->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &pDSV));
	
	pContext->OMSetRenderTargets(1u, pTarget.GetAddressOf(), pDSV.Get());
}

Graphics::~Graphics()
{
	
}

void Graphics::EndFrame()
{
	DrawTestTriangle();
	pSwap->Present(1, 0u);
}


void Graphics::DrawTestTriangle()
{
	HRESULT hr;
	struct Vertex
	{
		struct {

			float x;
			float y;
			float z;
		}pos;
		struct  
		{
			float r;
			float g;
			float b;
		}color;
	};


	const Vertex verticies[]
	{
		// positions      // texture coords

		//front
		{-1.0f, -1.0f, -1.0f, 255.0f, 0.0f,0.0f},    // top right
		{1.0f, -1.0f, -1.0f, 0.0f, 255.0f,0.0f},   // bottom right
		{-1.0f, 1.0f, -1.0f, 0.0f, 0.0f,255.0f},  // bottom left
		{1.0f, 1.0f, -1.0f, 255.0f, 255.0f,0.0},   // top left 

		//back
		{1.0f, 1.0f, 1.0f, 255.0f, 0.0f,0.0f},   // top right
		{1.0f, -1.0f, 1.0f, 255.0f, 0.0f,0.0f},  // bottom right
		{-1.0f, -1.0f, 1.0f, 255.0f, 0.0f,0.0f}, // bottom left
		{-1.0f, 1.0f, 1.0f, 255.0f, 0.0f,0.0f},  // top left 
	};
	const unsigned short indices[] =
	{
		// front
	  0, 1, 3,
	  1, 2, 3,
	  // back
	  4, 5, 7,
	  5, 6, 7,
	  // right
	  0, 1, 4,
	  1, 4, 5,
	  // left
	  2, 3, 7,
	  2, 6, 7,
	  // top
	  0, 3, 4,
	  3, 4, 7,
	  // bottom
	  1, 2, 5,
	  2, 5, 6
	};


	Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;
	D3D11_BUFFER_DESC IndexBufferDesc = {};
	IndexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	IndexBufferDesc.CPUAccessFlags = 0u;
	IndexBufferDesc.MiscFlags = 0u;
	IndexBufferDesc.ByteWidth = sizeof(indices);
	IndexBufferDesc.StructureByteStride = sizeof(unsigned short);

	D3D11_SUBRESOURCE_DATA isd = {};
	isd.pSysMem = indices;
	GFX_EXCEPT_NOINFO(pDevice->CreateBuffer(&IndexBufferDesc, &isd, &pIndexBuffer));
	 pContext->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u );

	Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
	D3D11_BUFFER_DESC VertexDesc = {};
	VertexDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexDesc.Usage = D3D11_USAGE_DEFAULT;
	VertexDesc.CPUAccessFlags = 0u;
	VertexDesc.MiscFlags = 0u;
	VertexDesc.ByteWidth = sizeof(verticies);
	VertexDesc.StructureByteStride = sizeof(Vertex);

	D3D11_SUBRESOURCE_DATA sd = {};
	sd.pSysMem = verticies;
	GFX_EXCEPT_NOINFO(pDevice->CreateBuffer(&VertexDesc, &sd, &pVertexBuffer));
	CONST UINT stride = sizeof(Vertex);
	const UINT offset = 0u;

	
	pContext->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &stride, &offset);	
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	const D3D11_INPUT_ELEMENT_DESC ied[] = 
	{
		{"Position", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"Color", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,12u,D3D11_INPUT_PER_VERTEX_DATA,0},

	};
	
	GFX_EXCEPT_NOINFO(D3DReadFileToBlob(L"VertexShader.cso", &pBlob));
	GFX_EXCEPT_NOINFO(pDevice->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pVertexShader));
	
	pContext->VSSetShader(pVertexShader.Get(),nullptr,0u);
	
	GFX_EXCEPT_NOINFO(pDevice->CreateInputLayout(
		ied, (UINT)std::size(ied),
		pBlob->GetBufferPointer(),
		pBlob->GetBufferSize(),
		&pInputLayout
	));

	pContext->IASetInputLayout(pInputLayout.Get());

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;
	GFX_EXCEPT_NOINFO(D3DReadFileToBlob(L"PixelShader.cso", &pBlob));
	GFX_EXCEPT_NOINFO(pDevice->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pPixelShader));

	pContext->PSSetShader(pPixelShader.Get(), nullptr, 0u);


	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3D11_VIEWPORT vp;
	vp.Width = 800;
	vp.Height = 600;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	pContext->RSSetViewports(1u, &vp);

	struct ConstantBuffer
	{
		DirectX::XMMATRIX transform;

	};
	const ConstantBuffer cb = 
	{
		DirectX::XMMatrixTranspose(
		DirectX::XMMatrixTranslation(0.0f,0.0f,4.0f) *
		DirectX::XMMatrixRotationZ(30.0f)*
		DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f/4.0f,0.5f,10.0f)
		)
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;
	D3D11_BUFFER_DESC ConstDesc = {};
	ConstDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	ConstDesc.Usage = D3D11_USAGE_DYNAMIC;
	ConstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	ConstDesc.MiscFlags = 0u;
	ConstDesc.ByteWidth = sizeof(cb);
	ConstDesc.StructureByteStride = sizeof(ConstantBuffer);

	D3D11_SUBRESOURCE_DATA csd = {};
	csd.pSysMem = &cb;
	GFX_EXCEPT_NOINFO(pDevice->CreateBuffer(&ConstDesc, &csd, &pConstantBuffer));

	pContext->VSSetConstantBuffers(0u, 1u, pConstantBuffer.GetAddressOf());

	pContext->DrawIndexed((UINT)std::size(indices), 0u, 0u);
}

// Graphics exception stuff
Graphics::HrException::HrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file),
	hr(hr)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if (!info.empty())
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "DOE Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	std::wstring ws = DXGetErrorString(hr);
	return std::string(ws.begin(), ws.end());
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription(hr, ((WCHAR*)(buf)), sizeof(buf));
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
Graphics::InfoException::InfoException(int line, const char* file, std::vector<std::string> infoMsgs) noexcept
	:
	Exception(line, file)
{
	// join all info messages with newlines into single string
	for (const auto& m : infoMsgs)
	{
		info += m;
		info.push_back('\n');
	}
	// remove final newline if exists
	if (!info.empty())
	{
		info.pop_back();
	}
}


const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "DOE Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}
