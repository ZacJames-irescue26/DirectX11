#pragma once
#include "AnimatedMesh.h"
#include <thread>
#include <shared_mutex>
#include "Math\Triangle.h"
#include "BoneData.h"
#include "AnimationInfo.h"
#include "Skeleton.h"
#include "ConstantBufferTypes.h"
#include "src/Graphics/ConstantBuffer.h"
namespace Engine
{
	

	class AnimatedModel
	{
	public:
		bool Initialize(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* devicecontext, ConstantBuffer<CB_Anim_VS_vertexShader>& cb_vs_vertexshader);
		void UpdateAnimation(float deltaTime);
		void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix);
		void Draw();
		inline std::vector<Engine::AnimatedMesh>& GetMeshes()
		{
			return meshes;
		}
		inline const Skeleton& GetSkeleton()
		{
			return m_Skeleton;
		}
		void LoadAnimationOnly(const std::string& path);
		bool LoadModel(const std::string& filePath);

		void RemoveAnimation(uint32_t index)
		{
			LoadedAnimations.erase(LoadedAnimations.begin() + index);
		}
		void RemoveAnimationByName(const std::string& name)
		{
			for (int i = 0; i < LoadedAnimations.size(); i++)
			{
				if (LoadedAnimations[i].Name == name)
				{
					LoadedAnimations.erase(LoadedAnimations.begin() + i);
					return;
				}
			}
		}
		const std::vector<AnimationClip>& GetLoadedAnimations()
		{
			return LoadedAnimations;
		}
		int GetCurrentAnimationIndex() const
		{
			return m_CurrentAnimationIndex;
		}

		void SetCurrentAnimationIndex(int index)
		{
			if (index >= 0 && index < static_cast<int>(LoadedAnimations.size()))
				m_CurrentAnimationIndex = index;
		}

		void SetPlayback(float playback)
		{
			m_PlaybackSpeed = playback;
		}
	private:
		void ProcessNode(aiNode* node, const aiScene* scene, const XMMATRIX& transformMatrix);
		void ProcessMesh(aiMesh* mesh, const aiScene* scene, const XMMATRIX& transformMatrix);
		void CreateMesh(std::vector<AnimatedVertex> vertices, std::vector<DWORD> indices, std::vector<Texture> tex, const XMMATRIX transformMatrix);
		TextureStorageType DetermineTextureStorageType(const aiScene* pScene, aiMaterial* pMat, unsigned int index, aiTextureType textureType);
		std::vector<Texture> LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType textureType, const aiScene* pScene);
		int GetTextureIndex(aiString* pStr);
	


		ID3D11Device* device = nullptr;
		ID3D11DeviceContext* deviceContext = nullptr;
		ConstantBuffer<CB_Anim_VS_vertexShader>* cb_vs_vertexshader = nullptr;
		
		std::vector<AnimatedMesh> meshes;
		std::string directory = "";
		std::vector<Engine::Triangle> ExtractTriangles();
		XMMATRIX m_GlobalInverseTransform;
		std::vector<AnimationClip> LoadedAnimations;
		const aiScene* pScene;
		Assimp::Importer importer;
		Skeleton m_Skeleton;
		float m_AnimationTime = 0.0f;
		float m_PlaybackSpeed = 1.0f;
		int m_CurrentAnimationIndex = -1;
		bool PlayAnimaiton = true;

	};
}