#include "pch.h"
#include "Model.h"


#include "ErrorLogger.h"
#include "Graphics.h"

Model::Model()
{

}

Model::CompleteModel Model::Initialize(ID3D11Device* device, const std::string& filepath)
{
	if (!IsInitialized)
	{
		LoadModel(filepath);
		CompleteModel model = (ConstructIndexAndVertexBuffer(ModelRawData));
		IsInitialized = true;
		return model;
	}
	else
	{
		ErrorLogger::Log("Model already initialized check for reinit");
	}


}

void Model::LoadModel(const std::string& path)
{
	// read file via ASSIMP
	Assimp::Importer importer;
	const auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
	// check for errors

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
	}

	// retrieve the directory path of the filepath
	/*if (scene->GetEmbeddedTexture(path.c_str()) == nullptr)
	{
		Texture->directory = path.substr(0, path.find_last_of('\\'));
		if (Modelmaterial->directory == m_path)
		{
			Modelmaterial->directory = path.substr(0, path.find_last_of('/'));

		}
	}
	else
	{
		const aiTexture* texture = scene->GetEmbeddedTexture(path.c_str());

		Modelmaterial->LoadEmbeddedTexture(texture, texture->mFilename.C_Str());
	}*/


	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode* node, const aiScene* scene)
{

	// process each mesh located at the current node
	//assert(node->mNumMeshes != 0 && "Number of meshes is 0: %i", node->mNumMeshes);
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

		ModelRawData.push_back(processMesh(mesh, scene, i));

	}
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}

}

Model::RawData Model::processMesh(aiMesh* mesh, const aiScene* scene, int mesh_index)
{
	// data to fill
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	//std::vector<Texture> textures;
	// walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		XMFLOAT3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
		// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.pos = vector;
		// normals
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.normal = vector;
		}
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			XMFLOAT2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.texCoord = vec;
		}
		else
		{
			vertex.texCoord = XMFLOAT2(0.0f, 0.0f);
		}
		/*if (mesh->mTangents)
		{
			vector.x = mesh->mTangents[i].x;
			vector.y = mesh->mTangents[i].y;
			vector.z = mesh->mTangents[i].z;
			vertex.Tangent = vector;
		}
		else
		{
			vertex.Tangent = glm::vec3(1.0f);
		}
		if (mesh->mBitangents)
		{
			vector.x = mesh->mBitangents[i].x;
			vector.y = mesh->mBitangents[i].y;
			vector.z = mesh->mBitangents[i].z;
			vertex.Bitangent = vector;
		}
		else
		{
			vertex.Bitangent = glm::vec3(1.0f);
		}*/
		vertices.push_back(vertex);
	}
	// now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN
	//// 1. diffuse maps
	//Modelmaterial->loadMaterialTextures(material, aiTextureType_DIFFUSE, TextureType::Diffuse);
	//// 2. specular maps
	//Modelmaterial->loadMaterialTextures(material, aiTextureType_SPECULAR, TextureType::Specular);

	//// 3. normal maps
	//Modelmaterial->loadMaterialTextures(material, aiTextureType_HEIGHT, TextureType::Normal);

	//// 4. height maps
	//Modelmaterial->loadMaterialTextures(material, aiTextureType_AMBIENT, TextureType::Height);
	//// 5.0 metalness
	//Modelmaterial->loadMaterialTextures(material, aiTextureType_METALNESS, TextureType::Metalness);
	ID3D11Device* device = Graphics::GetDevice();
	TextureMap mapdiffuse = LoadTextureToShaderResource(device, material, aiTextureType_DIFFUSE, TextureType::Diffuse);
	TextureMap mapnorm = LoadTextureToShaderResource(device, material, aiTextureType_DIFFUSE, TextureType::Normal);
	TextureMap mapheight = LoadTextureToShaderResource(device, material, aiTextureType_DIFFUSE, TextureType::Height);
	TextureMap mapspec = LoadTextureToShaderResource(device, material, aiTextureType_DIFFUSE, TextureType::Specular);

	TextureMap total;
	total.insert(mapdiffuse.begin(), mapdiffuse.end());
	total.insert(mapnorm.begin(), mapnorm.end());
	total.insert(mapheight.begin(), mapheight.end());
	total.insert(mapspec.begin(), mapspec.end());
	mapdiffuse.empty();
	mapnorm.empty();
	mapheight.empty();
	mapspec.empty();

	//if (mesh->HasBones())
	//{
	//	ExtractBoneWeightForVertices(vertices, mesh, scene);
	//}
	//// return a mesh object created from the extracted mesh data
	return Model::RawData(vertices, indices, mesh->mName.C_Str(), total);
}

std::vector<Model::LoadedModel>& Model::ConstructIndexAndVertexBuffer(std::vector<Model::RawData> data)
{
	std::vector<Model::LoadedModel> model;
	VertexBuffer<Vertex> vert;
	IndexBuffer index;

	for (auto& rawdata : data)
	{
		HRESULT hr = vert.Initialize(Graphics::GetDevice(), rawdata.vertices.data(), rawdata.vertices.size());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to create vertex buffer.");
		}
		//Load Index Data
		hr = index.Initialize(Graphics::GetDevice(), rawdata.indices.data(), rawdata.indices.size());
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to create indices buffer.");
		}
		Model::LoadedModel partmodel = {vert, index, rawdata.textureMap, rawdata.name};

		model.push_back(partmodel);

	}
	return model;
}

TextureMap Model::LoadTextureToShaderResource(ID3D11Device* device, aiMaterial* mat, aiTextureType textype, TextureType type)
{


	TextureMap map;
	map.empty();
	aiString path;
	SamplerState state;
	ShaderResource tex;
	for (int i = 0; i < mat->GetTextureCount(textype); i++)
	{
		if (i > 0)
		{
			ErrorLogger::Log("More than one texture isnt supported yet per type");
			break;
		}
		state.Reset();
		tex.Reset();
		mat->GetTexture(textype, i, &path);
		std::string pathstr = "Assets/" + std::string(path.C_Str());
		//Create sampler description for sampler state
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		HRESULT hr = device->CreateSamplerState(&sampDesc, state.GetAddressOf()); //Create sampler state
		if (FAILED(hr))
		{
			ErrorLogger::Log(hr, "Failed to create sampler state.");
			return{};
		}
		// load image
		unsigned char* data = nullptr;
		int img_width, img_height, img_channels;
		{
			data = stbi_load(pathstr.c_str(), &img_width, &img_height, &img_channels, 4);
			if (data == nullptr)
			{
				ErrorLogger::Log("Failed to load image\n");
				return {};
			}
		}
		// craete texture
		Microsoft::WRL::ComPtr<ID3D11Texture2D> comtexture = nullptr;
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

			hr = device->CreateTexture2D(&texture_desc, &subresource_data, comtexture.GetAddressOf());
			if (FAILED(hr))
			{
				ErrorLogger::Log(hr, L"Failed to create texture 2d\n");
				return {};

			}
			stbi_image_free(data);
		}

		// create texture view
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC view_desc = {};
			view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			view_desc.Texture2D.MipLevels = 1;
			hr = device->CreateShaderResourceView(comtexture.Get(), &view_desc, tex.GetAddressOf());
			if (FAILED(hr))
			{
				ErrorLogger::Log(hr, L"Failed to create render target view\n");
				return {};

			}
		}
		comtexture->Release();
		Texture texture = { path.C_Str(), type, state, tex };
		map.emplace(type, texture);
	}
		return map;
}