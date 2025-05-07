#pragma once
#include "Color.h"
#include <assimp/material.h>
namespace Engine
{
enum class TextureStorageType
{
	Invalid,
	None,
	EmbeddedIndexCompressed,
	EmbeddedIndexNonCompressed,
	EmbeddedCompressed,
	EmbeddedNonCompressed,
	Disk
};

class Texture
{
public:
	Texture(nvrhi::DeviceHandle device, const Color& color, aiTextureType type);
	Texture(nvrhi::DeviceHandle device, const Color* colorData, UINT width, UINT height, aiTextureType type); //Generate texture of specific color data
	Texture(nvrhi::DeviceHandle device, nvrhi::CommandListHandle deviceContext, const std::string& filePath, aiTextureType type);
	//Texture(ID3D11Device* device, const uint8_t* pData, size_t size, aiTextureType type);
	
	Texture(nvrhi::DeviceHandle device, aiTexture* texture, size_t size, aiTextureType type);
	aiTextureType GetType();
	nvrhi::TextureHandle GetRawTexture();

private:
	void Initialize1x1ColorTexture(nvrhi::DeviceHandle device, const Color& colorData, aiTextureType type);
	void InitializeColorTexture(nvrhi::DeviceHandle device, const Color* colorData, UINT width, UINT height, aiTextureType type);
	nvrhi::TextureHandle texture = nullptr;
	aiTextureType type = aiTextureType::aiTextureType_UNKNOWN;
};
}
