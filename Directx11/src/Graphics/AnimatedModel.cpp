#include "pch.h"
#include "AnimatedModel.h"
#include "ErrorLogger.h"
#include "ConstantBuffer.h"
#include "Color.h"


static std::shared_mutex ModelWriteMeshMutex;

namespace Engine
{

	bool AnimatedModel::Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* devicecontext, ConstantBuffer<CB_Anim_VS_vertexShader>& cb_vs_vertexshader)
	{
		this->device = device;
		this->deviceContext = devicecontext;
		this->cb_vs_vertexshader = &cb_vs_vertexshader;

		try
		{
			if (!LoadModel(filePath))
				return false;
		}
		catch (COMException& exception)
		{
			ErrorLogger::Log(exception);
			return false;
		}
		return true;
	}
	void AnimatedModel::UpdateAnimation(float deltaTime)
	{
		if (m_CurrentAnimationIndex < 0 ||
			m_CurrentAnimationIndex >= static_cast<int>(LoadedAnimations.size()))
		{
			return;
		}

		AnimationClip& clip =
			LoadedAnimations[m_CurrentAnimationIndex];

		const double ticksPerSecond =
			clip.TicksPerSecond > 0.0
			? clip.TicksPerSecond
			: 25.0;

		const float durationSeconds =
			static_cast<float>(
				clip.Duration / ticksPerSecond
				);

		if (durationSeconds <= 0.0f)
			return;

		m_AnimationTime += deltaTime * m_PlaybackSpeed;

		if (m_Looping)
		{
			m_AnimationTime =
				std::fmod(m_AnimationTime, durationSeconds);

			if (m_AnimationTime < 0.0f)
				m_AnimationTime += durationSeconds;
		}
		else
		{
			m_AnimationTime =
				std::clamp(
					m_AnimationTime,
					0.0f,
					durationSeconds
				);
		}

		m_Skeleton.EvaluateAnimationAtTime(
			m_AnimationTime,
			&clip
		);
	}

	bool AnimatedModel::HasAnimationFinished() const
	{
		if (m_CurrentAnimationIndex < 0 ||
			m_CurrentAnimationIndex >= static_cast<int>(LoadedAnimations.size()))
		{
			return true;
		}

		const AnimationClip& clip =
			LoadedAnimations[m_CurrentAnimationIndex];

		const double ticksPerSecond =
			clip.TicksPerSecond > 0.0
			? clip.TicksPerSecond
			: 25.0;

		const float durationSeconds =
			static_cast<float>(
				clip.Duration / ticksPerSecond
				);

		return !m_Looping &&
			m_AnimationTime >= durationSeconds;
	}

	void AnimatedModel::Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
	{
		if (!cb_vs_vertexshader)
			return;

		deviceContext->VSSetConstantBuffers(0,1,cb_vs_vertexshader->GetAddressOf());

		cb_vs_vertexshader->data.wvpMatrix = XMMatrixTranspose(worldMatrix * viewProjectionMatrix);

		cb_vs_vertexshader->data.worldMatrix = XMMatrixTranspose(worldMatrix);

		cb_vs_vertexshader->data.worldInvTransposeMatrix = XMMatrixTranspose(XMMatrixInverse(nullptr, worldMatrix));
		

		const auto& bones = m_Skeleton.GetBones();

		constexpr size_t MaxBones = 100;
		
		for (size_t i = 0; i < MaxBones; ++i)
		{
			if (i < bones.size())
			{
				cb_vs_vertexshader->data.Bones[i] =
					XMMatrixTranspose(bones[i].FinalTransformation);
			}
			else
			{
				cb_vs_vertexshader->data.Bones[i] =
					XMMatrixIdentity();
			}
		}

		cb_vs_vertexshader->ApplyChanges();

		for (auto& mesh : meshes)
		{
			mesh.Draw();
		}
	}
	void AnimatedModel::Draw()
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			//Update Constant buffer with WVP Matrix

			meshes[i].DrawJustMesh();
		}
	}
	bool AnimatedModel::LoadModel(const std::string& filePath)
	{

		directory = StringHelper::GetDirectoryFromPath(filePath);
		

		pScene = importer.ReadFile(filePath,
			aiProcess_Triangulate |
			aiProcess_ConvertToLeftHanded);

		if (pScene == nullptr)
			return false;


		m_Skeleton.BuildFromScene(pScene);

		this->ProcessNode(pScene->mRootNode, pScene, XMMatrixIdentity());
		
		
		
		return true;
	}

	void AnimatedModel::ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& parentTransformMatrix)
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

	void AnimatedModel::ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix)
	{
		std::vector<AnimatedVertex> vertices(mesh->mNumVertices);
		std::vector<DWORD> indices;
		std::vector<VertexBoneData> vertexBoneData(mesh->mNumVertices);
		for (UINT i = 0; i < mesh->mNumVertices; i++)
		{
			AnimatedVertex vertex{};

			vertex.pos.x = mesh->mVertices[i].x;
			vertex.pos.y = mesh->mVertices[i].y;
			vertex.pos.z = mesh->mVertices[i].z;

			if (mesh->mTextureCoords[0])
			{
				vertex.texCoord.x = mesh->mTextureCoords[0][i].x;
				vertex.texCoord.y = mesh->mTextureCoords[0][i].y;
			}

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

			vertices[i] = vertex;
		}

		for (UINT i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (UINT j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			aiBone* bone = mesh->mBones[boneIndex];

			int boneId = m_Skeleton.GetBoneId(bone);

			for (uint32_t weightIndex = 0; weightIndex < bone->mNumWeights; ++weightIndex)
			{
				const aiVertexWeight& weight = bone->mWeights[weightIndex];

				uint32_t vertexId = weight.mVertexId;

				if (vertexId >= vertexBoneData.size())
				{
					OutputDebugStringA("ERROR: Bone weight vertex index out of range\n");
					__debugbreak();
					continue;
				}

				vertexBoneData[vertexId].AddBoneData(
					static_cast<uint32_t>(boneId),
					weight.mWeight
				);
			}
		}

		for (size_t i = 0; i < vertices.size(); ++i)
		{
			vertexBoneData[i].SortByWeightDesc();
			vertexBoneData[i].Normalize();

			for (int j = 0; j < MAXBONEPERVERTEX; ++j)
			{
				if (vertexBoneData[i].BoneIDs[j] >= 100)
				{
					OutputDebugStringA("ERROR: Bone ID exceeds Bones[100]\n");
					__debugbreak();
				}

				vertices[i].m_BoneIDs[j] = vertexBoneData[i].BoneIDs[j];
				vertices[i].m_Weights[j] = vertexBoneData[i].Weights[j];
			}
		}

		std::vector<Texture> textures;
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		auto diffuseTextures =
			LoadMaterialTextures(material, aiTextureType_DIFFUSE, scene);

		auto normalTextures =
			LoadMaterialTextures(material, aiTextureType_NORMALS, scene);

		auto roughnessTextures =
			LoadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS, scene);

		textures.insert(textures.end(), diffuseTextures.begin(), diffuseTextures.end());
		textures.insert(textures.end(), normalTextures.begin(), normalTextures.end());
		textures.insert(textures.end(), roughnessTextures.begin(), roughnessTextures.end());

		CreateMesh(vertices, indices, textures, transformMatrix);
	}

	void AnimatedModel::CreateMesh(std::vector<AnimatedVertex> vertices, std::vector<DWORD> indices, std::vector<Texture> tex, const XMMATRIX transformMatrix)
	{
		std::lock_guard<std::shared_mutex> lck_guard(ModelWriteMeshMutex);
		meshes.emplace_back(this->device, this->deviceContext, vertices, indices, tex, transformMatrix);


	}

	TextureStorageType AnimatedModel::DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType)
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



	std::vector<Texture> AnimatedModel::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene)
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

	int AnimatedModel::GetTextureIndex(aiString* pStr)
	{
		assert(pStr->length >= 2);
		return atoi(&pStr->C_Str()[1]);
	}

	void AnimatedModel::LoadAnimationOnly(
		const std::string& path,
		const std::string& name)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(
			path,
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices |
			aiProcess_LimitBoneWeights
		);

		if (!scene || !scene->HasAnimations())
		{
			ErrorLogger::Log("Invalid path or no animations");
			return;
		}

		aiAnimation* aiAnim = scene->mAnimations[0];

		AnimationClip clip;
		clip.filepath = path;
		clip.Name = name;
		clip.Duration = aiAnim->mDuration;
		clip.TicksPerSecond =
			aiAnim->mTicksPerSecond != 0.0 ? aiAnim->mTicksPerSecond : 25.0;

		for (uint32_t i = 0; i < aiAnim->mNumChannels; i++)
		{
			aiNodeAnim* channel = aiAnim->mChannels[i];

			std::string boneName = channel->mNodeName.C_Str();

			// Only keep channels that exist in your skeleton.
			if (!m_Skeleton.HasNode(boneName))
				continue;

			AnimationChannel outChannel;
			outChannel.BoneName = boneName;

			for (uint32_t k = 0; k < channel->mNumPositionKeys; k++)
			{
				const aiVectorKey& key = channel->mPositionKeys[k];

				outChannel.Positions.push_back({
					(float)key.mTime,
					XMFLOAT3(
						key.mValue.x,
						key.mValue.y,
						key.mValue.z
					)
					});
			}

			for (uint32_t k = 0; k < channel->mNumRotationKeys; k++)
			{
				const aiQuatKey& key = channel->mRotationKeys[k];

				outChannel.Rotations.push_back({
					(float)key.mTime,
					XMFLOAT4(
						key.mValue.x,
						key.mValue.y,
						key.mValue.z,
						key.mValue.w
					)
					});
			}

			for (uint32_t k = 0; k < channel->mNumScalingKeys; k++)
			{
				const aiVectorKey& key = channel->mScalingKeys[k];

				outChannel.Scales.push_back({
					(float)key.mTime,
					XMFLOAT3(
						key.mValue.x,
						key.mValue.y,
						key.mValue.z
					)
					});
			}

			clip.Channels.push_back(std::move(outChannel));
		}
		LoadedAnimations.push_back(clip);
	}

}