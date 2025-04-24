#pragma once
#include "Mesh.h"
#include <thread>
#include <shared_mutex>

namespace Engine
{
class Model
{
public:
	bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader);
	void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix);
	void Draw();
	inline std::vector<Mesh>& GetMeshes()
	{
		return meshes;
	}
private:
	std::vector<Mesh> meshes;
	bool LoadModel(const std::string& filePath);
	void ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& transformMatrix);
	void ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix);
	void CreateMesh(std::vector<Vertex> vertices, std::vector<DWORD> indices, std::vector<Texture> tex, const XMMATRIX transformMatrix); 

	TextureStorageType DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
	std::vector<Texture> LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);

	int GetTextureIndex(aiString* pStr);
	ID3D11Device* device = nullptr;
	ID3D11DeviceContext* deviceContext = nullptr;
	ConstantBuffer<CB_VS_vertexShader>* cb_vs_vertexshader = nullptr;
	std::string directory = "";
	

};
}