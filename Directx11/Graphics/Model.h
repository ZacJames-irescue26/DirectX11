#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Vertex.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


typedef Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState;
typedef Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ShaderResource;

enum TextureType
{
	Diffuse,
	Normal,
	Specular,
	Height
};

//to be moved to its own file
struct Texture {
public:
	Texture(const std::string& path, TextureType type, SamplerState state, ShaderResource texture)
	:path(path), type(type), state(state), texture(texture)
	{};
	SamplerState state;
	ShaderResource texture;
	TextureType type;
	const std::string& path;
};

typedef std::map<TextureType, Texture> TextureMap;


class Model
{
public:
	struct LoadedModel;
	
public:
	Model();
	typedef std::vector<LoadedModel> CompleteModel;
	CompleteModel Initialize(ID3D11Device* device, const std::string& filepath);
	struct RawData
	{
		RawData(std::vector<Vertex> verts, std::vector<DWORD> indi, const std::string& name, TextureMap map)
		: vertices(verts), indices(indi), name(name), textureMap(map)
		{}
		std::vector<Vertex> vertices;
		std::vector<DWORD> indices;
		TextureMap textureMap;
		
		const std::string& name;
	};
	struct LoadedModel
	{
	public:

		LoadedModel(VertexBuffer<Vertex>& vert, IndexBuffer& index, TextureMap map, const std::string& name)
		: m_vertexBuffer(vert), m_indexBuffer(index), textureMap(map), name(name)
		{}
		VertexBuffer<Vertex>& m_vertexBuffer;
		IndexBuffer& m_indexBuffer;
		TextureMap textureMap;
		const std::string& name;

	};
private:


	void LoadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);
	Model::RawData processMesh(aiMesh* mesh, const aiScene* scene, int mesh_index);
	std::vector<LoadedModel>& ConstructIndexAndVertexBuffer(std::vector<Model::RawData> data);
	TextureMap LoadTextureToShaderResource(ID3D11Device* device, aiMaterial* mat, aiTextureType textype, TextureType type);
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	
	std::vector<LoadedModel> m_Model;
	std::vector<Model::RawData> ModelRawData;

	bool IsInitialized = false;
};

