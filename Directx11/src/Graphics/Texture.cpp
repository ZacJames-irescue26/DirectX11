#include "pch.h"
#include "Texture.h"
#include "stb_image.h"
#include "..\\ErrorLogger.h"
#include "nvrhi\nvrhi.h"
namespace Engine
{
Texture::Texture(nvrhi::DeviceHandle device, const Color& color, aiTextureType type)
{
	this->Initialize1x1ColorTexture(device, color, type);
}

Texture::Texture(nvrhi::DeviceHandle device, const Color* colorData, UINT width, UINT height, aiTextureType type)
{
	this->InitializeColorTexture(device, colorData, width, height, type);
}

Texture::Texture(nvrhi::DeviceHandle device, nvrhi::CommandListHandle deviceContext, const std::string& filePath, aiTextureType type)
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

	{

		nvrhi::TextureDesc texture_desc = {};
		texture_desc.width = img_width;
		texture_desc.height = img_height;
		texture_desc.arraySize = 1;
	if (type == aiTextureType_DIFFUSE)
	{
		
		texture_desc.format = nvrhi::Format::SRGBA8_UNORM;
	}
	else if( type == aiTextureType_NORMALS)
	{
		texture_desc.format = nvrhi::Format::RGBA8_UNORM;
	}
	else if (type == aiTextureType_DIFFUSE_ROUGHNESS)
	{
		texture_desc.format = nvrhi::Format::RGBA8_UNORM;
	}
		texture_desc.isShaderResource = true;
		texture_desc.isRenderTarget = true;
		texture_desc.mipLevels = 0;
		
		//D3D11_SUBRESOURCE_DATA subresource_data = {};
		//subresource_data.pSysMem = data;
		//subresource_data.SysMemPitch = img_width * 4;
		texture = device->createTexture(texture_desc);
		deviceContext->writeTexture(texture,1,0, data, img_width*4);

	
	}
	stbi_image_free(data);
}

Texture::Texture(nvrhi::DeviceHandle* device, aiTexture* intexture, size_t size, aiTextureType type)
{
//	this->type = type;
//
//
//	// load image
//	unsigned char* data = nullptr;
//	int img_width, img_height, img_channels;
//	{
//	if (intexture->mHeight == 0)
//	{
//		data = stbi_load_from_memory(reinterpret_cast<unsigned char*> (intexture->pcData), intexture->mWidth, &img_width, &img_height, &img_channels, 4);
//	}
//	else
//	{
//		data = stbi_load_from_memory(reinterpret_cast<unsigned char*> (intexture->pcData), intexture->mWidth * intexture->mHeight, &img_width, &img_height, &img_channels, 4);
//
//	}
//		if (data == nullptr)
//		{
//			ErrorLogger::Log("Failed to load image\n");
//		}
//	}
//
//	// create texture
//	Microsoft::WRL::ComPtr<ID3D11Texture2D> tex = nullptr;
//	{
//		D3D11_TEXTURE2D_DESC texture_desc = {};
//		texture_desc.Width = img_width;
//		texture_desc.Height = img_height;
//		texture_desc.MipLevels = 1;
//		texture_desc.ArraySize = 1;
//		texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//		texture_desc.SampleDesc.Count = 1;
//		texture_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
//		texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
//		texture_desc.MipLevels = 0; // 0 = generate all mip levels
//		D3D11_SUBRESOURCE_DATA subresource_data = {};
//		subresource_data.pSysMem = data;
//		subresource_data.SysMemPitch = img_width * 4;
//
//		HRESULT hr = device->CreateTexture2D(&texture_desc, &subresource_data, tex.GetAddressOf());
//		if (FAILED(hr))
//		{
//			ErrorLogger::Log(hr, L"Failed to create texture 2d\n");
//
//		}
//		
//	}
//
//	// create texture view
//	{
//		D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
//		view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//		view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
//		view_desc.Texture2D.MipLevels = 1;
//		HRESULT hr = device->CreateShaderResourceView(tex.Get(), &view_desc, textureView.GetAddressOf());
//
//	COM_ERROR_IF_FAILED(hr, "Failed to create Texture from memory.");
//	}
//	tex->Release();
//	texture = tex;
}

aiTextureType Texture::GetType()
{
	return this->type;
}

nvrhi::TextureHandle Texture::GetRawTexture()
{
	return texture
}


void Texture::Initialize1x1ColorTexture(nvrhi::DeviceHandle device, const Color& colorData, aiTextureType type)
{
	InitializeColorTexture(device, &colorData, 1, 1, type);
}

void Texture::InitializeColorTexture(nvrhi::DeviceHandle device, const Color* colorData, UINT width, UINT height, aiTextureType type)
{
	/*this->type = type;
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
}*/
}