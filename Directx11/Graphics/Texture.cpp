#include "pch.h"
#include "Texture.h"
#include "stb_image.h"
#include "..\\ErrorLogger.h"
namespace Engine
{
Texture::Texture(ID3D11Device* device, const Color& color, aiTextureType type)
{
	this->Initialize1x1ColorTexture(device, color, type);
}

Texture::Texture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type)
{
	this->InitializeColorTexture(device, colorData, width, height, type);
}

Texture::Texture(ID3D11Device* device, const std::string& filePath, aiTextureType type)
{
	this->type = type;

	// load image
	unsigned char* data = nullptr;
	int img_width, img_height, img_channels;
	{
		data = stbi_load(filePath.c_str(), &img_width, &img_height, &img_channels, 4);
		if (data == nullptr)
		{
			ErrorLogger::Log("Failed to load image\n");
		}
	}
	// create texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex = nullptr;
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width = img_width;
		texture_desc.Height = img_height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = data;
		subresource_data.SysMemPitch = img_width * 4;

		HRESULT hr = device->CreateTexture2D(&texture_desc, &subresource_data, tex.GetAddressOf());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, L"Failed to create texture 2d\n");

		}
		stbi_image_free(data);
	}

	// create texture view
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = 1;
		HRESULT hr = device->CreateShaderResourceView(tex.Get(), &view_desc, textureView.GetAddressOf());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, L"Failed to create render target view\n");

		}
	}
	texture = tex;
	tex->Release();
}

Texture::Texture(ID3D11Device* device, aiTexture* intexture, size_t size, aiTextureType type)
{
	this->type = type;


	// load image
	unsigned char* data = nullptr;
	int img_width, img_height, img_channels;
	{
	if (intexture->mHeight == 0)
	{
		data = stbi_load_from_memory(reinterpret_cast<unsigned char*> (intexture->pcData), intexture->mWidth, &img_width, &img_height, &img_channels, 4);
	}
	else
	{
		data = stbi_load_from_memory(reinterpret_cast<unsigned char*> (intexture->pcData), intexture->mWidth * intexture->mHeight, &img_width, &img_height, &img_channels, 4);

	}
		if (data == nullptr)
		{
			ErrorLogger::Log("Failed to load image\n");
		}
	}

	// create texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex = nullptr;
	{
		D3D11_TEXTURE2D_DESC texture_desc = {};
		texture_desc.Width = img_width;
		texture_desc.Height = img_height;
		texture_desc.MipLevels = 1;
		texture_desc.ArraySize = 1;
		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texture_desc.SampleDesc.Count = 1;
		texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = data;
		subresource_data.SysMemPitch = img_width * 4;

		HRESULT hr = device->CreateTexture2D(&texture_desc, &subresource_data, tex.GetAddressOf());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, L"Failed to create texture 2d\n");

		}
	}

	// create texture view
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
		view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		view_desc.Texture2D.MipLevels = 1;
		HRESULT hr = device->CreateShaderResourceView(tex.Get(), &view_desc, textureView.GetAddressOf());

	COM_ERROR_IF_FAILED(hr, "Failed to create Texture from memory.");
	}
	tex->Release();
	texture = tex;
}

aiTextureType Texture::GetType()
{
	return this->type;
}

ID3D11ShaderResourceView* Texture::GetTextureResourceView()
{
	return this->textureView.Get();
}

ID3D11ShaderResourceView** Texture::GetTextureResourceViewAddress()
{
	return this->textureView.GetAddressOf();
}

void Texture::Initialize1x1ColorTexture(ID3D11Device* device, const Color& colorData, aiTextureType type)
{
	InitializeColorTexture(device, &colorData, 1, 1, type);
}

void Texture::InitializeColorTexture(ID3D11Device* device, const Color* colorData, UINT width, UINT height, aiTextureType type)
{
	this->type = type;
	CD3D11_TEXTURE2D_DESC textureDesc(DXGI_FORMAT_R8G8B8A8_UNORM, width, height);
	ID3D11Texture2D* p2DTexture = nullptr;
	D3D11_SUBRESOURCE_DATA initialData{};
	initialData.pSysMem = colorData;
	initialData.SysMemPitch = width * sizeof(Color);
	HRESULT hr = device->CreateTexture2D(&textureDesc, &initialData, &p2DTexture);
	COM_ERROR_IF_FAILED(hr, "Failed to initialize texture from color data.");
	texture = static_cast<ID3D11Texture2D*>(p2DTexture);
	CD3D11_SHADER_RESOURCE_VIEW_DESC srvDesc(D3D11_SRV_DIMENSION_TEXTURE2D, textureDesc.Format);
	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, textureView.GetAddressOf());
	COM_ERROR_IF_FAILED(hr, "Failed to create shader resource view from texture generated from color data.");
}
}