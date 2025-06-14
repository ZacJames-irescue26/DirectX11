#include "pch.h"
#include "ModelSimple.h"
#include "ErrorLogger.h"
#include "ConstantBuffer.h"
#include "Color.h"

static std::shared_mutex ModelWriteMeshMutex;

namespace Engine
{
bool Model::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* devicecontext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader)
{
	this->device = device;
	this->deviceContext = devicecontext;
	this->cb_vs_vertexshader = &cb_vs_vertexshader;

	try
	{
		if (!this->LoadModel(filePath))
			return false;
	}
	catch (COMException& exception)
	{
		ErrorLogger::Log(exception);
		return false;
	}
	return true;
}

void Model::Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
{

	this->deviceContext->VSSetConstantBuffers(0, 1, this->cb_vs_vertexshader->GetAddressOf());
		this->cb_vs_vertexshader->data.wvpMatrix = worldMatrix * viewProjectionMatrix; //Calculate World-View-Projection Matrix
		this->cb_vs_vertexshader->data.worldMatrix = worldMatrix; //Calculate World
		this->cb_vs_vertexshader->data.wvpMatrix = XMMatrixTranspose(this->cb_vs_vertexshader->data.wvpMatrix);
		this->cb_vs_vertexshader->data.worldMatrix = XMMatrixTranspose(this->cb_vs_vertexshader->data.worldMatrix);		
		cb_vs_vertexshader->data.worldInvTransposeMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
		
		this->cb_vs_vertexshader->ApplyChanges();


	for (int i = 0; i < meshes.size(); i++)
	{
		//Update Constant buffer with WVP Matrix
		
		meshes[i].Draw();
	}
}
void Model::Draw()
{
	for (int i = 0; i < meshes.size(); i++)
	{
		//Update Constant buffer with WVP Matrix

		meshes[i].DrawJustMesh();
	}
}
bool Model::LoadModel(const std::string& filePath)
{
	
	directory = StringHelper::GetDirectoryFromPath(filePath);
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filePath,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return false;

	this->ProcessNode(pScene->mRootNode, pScene, DirectX::XMMatrixIdentity());
	return true;
}

void Model::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix)
{
	XMMATRIX nodeTransformMatrix = XMMatrixIdentity();
	std::vector<std::thread> threads;
	for (UINT i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	/*	threads.emplace_back([=]() {
		});*/
		this->ProcessMesh(mesh, scene, nodeTransformMatrix);
	}
	// Wait for threads to finish
	//for (auto& thread : threads)
	//{
	//	if (thread.joinable())
	//		thread.join();
	//}
	for (UINT i = 0; i < node->mNumChildren; i++)
	{
		this->ProcessNode(node->mChildren[i], scene, nodeTransformMatrix);
	}
}

void Model::ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix)
{
	// Data to fill
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;

	//Get vertices
	for (UINT i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vertex.pos.x = mesh->mVertices[i].x;
		vertex.pos.y = mesh->mVertices[i].y;
		vertex.pos.z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
			//vertex.texCoord.y = 1.0f - mesh->mTextureCoords[0][i].y;
			vertex.texCoord.y = mesh->mTextureCoords[0][i].y;

		}
		else
			vertex.texCoord = {0.0,0.0};
		if (mesh->HasNormals())
		{
		
			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;
		}

		if (mesh->HasTangentsAndBitangents())
		{
			vertex.Tangent.x = mesh->mTangents[i].x;
			vertex.Tangent.y = mesh->mTangents[i].y;
			vertex.Tangent.z = mesh->mTangents[i].z;

			vertex.BiTangent.x = mesh->mBitangents[i].x;
			vertex.BiTangent.y = mesh->mBitangents[i].y;
			vertex.BiTangent.z = mesh->mBitangents[i].z;
		}
		vertices.push_back(vertex);
	}

	//Get indices
	for (UINT i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	std::vector<Texture> textures;
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_DIFFUSE, scene);
	
	std::vector<Texture> HeightTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_NORMALS, scene);
	std::vector<Texture> RoughnessTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, scene);

	//std::vector<Texture> MetalnessTextures = LoadMaterialTextures(material, aiTextureType::aiTextureType_METALNESS, scene);
	textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
	textures.insert(textures.end(), HeightTextures.begin(), HeightTextures.end());
	textures.insert(textures.end(), RoughnessTextures.begin(), RoughnessTextures.end());
	//textures.insert(textures.end(), MetalnessTextures.begin(), MetalnessTextures.end());

	CreateMesh(vertices, indices, textures, transformMatrix);
}

void Model::CreateMesh(std::vector<Vertex> vertices, std::vector<DWORD> indices, std::vector<Texture> tex, const XMMATRIX transformMatrix)
{
	std::lock_guard<std::shared_mutex> lck_guard(ModelWriteMeshMutex);
	meshes.push_back(Mesh(this->device, this->deviceContext, vertices, indices, tex, transformMatrix));


}

TextureStorageType Model::DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType)
{
	if (pMat->GetTextureCount(textureType) == 0)
		return TextureStorageType::None;

	aiString path;
	pMat->GetTexture(textureType, index, &path);
	std::string texturePath = path.C_Str();
	//Check if texture is an embedded indexed texture by seeing if the file path is an index #
	if (texturePath[0] == '*')
	{
		if (pScene->mTextures[0]->mHeight == 0)
		{
			return TextureStorageType::EmbeddedIndexCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR INDEXED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedIndexNonCompressed;
		}
	}
	//Check if texture is an embedded texture but not indexed (path will be the texture's name instead of #)
	if (auto pTex = pScene->GetEmbeddedTexture(texturePath.c_str()))
	{
		if (pTex->mHeight == 0)
		{
			return TextureStorageType::EmbeddedCompressed;
		}
		else
		{
			assert("SUPPORT DOES NOT EXIST YET FOR EMBEDDED NON COMPRESSED TEXTURES!" && 0);
			return TextureStorageType::EmbeddedNonCompressed;
		}
	}
	//Lastly check if texture is a filepath by checking for period before extension name
	if (texturePath.find('.') != std::string::npos)
	{
		return TextureStorageType::Disk;
	}

	return TextureStorageType::None; // No texture exists
}



std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
{
	std::vector<Texture> materialTextures;
	TextureStorageType storetype = TextureStorageType::Invalid;
	unsigned int textureCount = pMaterial->GetTextureCount(textureType);

	if (textureCount == 0) //If there are no textures
	{
		storetype = TextureStorageType::None;
		aiColor3D aiColor(0.0f, 0.0f, 0.0f);
		switch (textureType)
		{


		case aiTextureType_DIFFUSE:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) //If color = black, just use grey
			{
				materialTextures.push_back(Texture(this->device, ErrorColors::UnhandledTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(this->device, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		case aiTextureType_HEIGHT:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) //If color = black, just use grey
			{
				materialTextures.push_back(Texture(this->device, ErrorColors::UnhandledTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(this->device, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		case aiTextureType_METALNESS:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) //If color = black, just use grey
			{
				materialTextures.push_back(Texture(this->device, ErrorColors::UnhandledTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(this->device, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		case aiTextureType_DIFFUSE_ROUGHNESS:
			pMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor);
			if (aiColor.IsBlack()) //If color = black, just use grey
			{
				materialTextures.push_back(Texture(this->device, ErrorColors::UnhandledTextureColor, textureType));
				return materialTextures;
			}
			materialTextures.push_back(Texture(this->device, Color(aiColor.r * 255, aiColor.g * 255, aiColor.b * 255), textureType));
			return materialTextures;
		}
	}
	else
	{
		for (UINT i = 0; i < textureCount; i++)
		{
			aiString path;
			pMaterial->GetTexture(textureType, i, &path);
			TextureStorageType storetype = DetermineTextureStorageType(pScene, pMaterial, i, textureType);
			switch (storetype)
			{

			case TextureStorageType::EmbeddedIndexCompressed:
			{
				int index = GetTextureIndex(&path);
				Texture embeddedIndexedTexture(this->device,
					/*reinterpret_cast<uint8_t*>*/(pScene->mTextures[index]),
					pScene->mTextures[index]->mWidth,
					textureType);
				materialTextures.push_back(embeddedIndexedTexture);
				break;
			}

			case TextureStorageType::EmbeddedCompressed:
			{
				//const aiTexture* pTexture = pScene->GetEmbeddedTexture(path.C_Str());
				//Texture embeddedTexture(this->device,
				//	/*reinterpret_cast<uint8_t*>*/(pTexture),
				//	pTexture->mWidth,
				//	textureType);
				//materialTextures.push_back(embeddedTexture);
				break;
			}

			case TextureStorageType::Disk:
			{
				std::string filename = this->directory + '\\' + path.C_Str();
				Texture diskTexture(this->device, deviceContext, filename, textureType);
				materialTextures.push_back(diskTexture);
				break;
			}
			}
		}
	}

	if (materialTextures.size() == 0)
	{
		materialTextures.push_back(Texture(this->device, ErrorColors::UnhandledTextureColor, aiTextureType::aiTextureType_DIFFUSE));
	}
	return materialTextures;
}

int Model::GetTextureIndex(aiString* pStr)
{
	assert(pStr->length >= 2);
	return atoi(&pStr->C_Str()[1]);
}

}