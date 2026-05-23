#pragma once


#pragma once

#include <DirectXMath.h>
#include <assimp/scene.h>
#include <assimp/anim.h>

#include <string>
#include <vector>
#include <unordered_map>
#include "BoneData.h"
#include "AnimationInfo.h"
namespace Engine
{
	inline DirectX::XMMATRIX AiMatrixToXMMATRIX(const aiMatrix4x4& m)
	{
		return DirectX::XMMATRIX(
			m.a1, m.a2, m.a3, m.a4,
			m.b1, m.b2, m.b3, m.b4,
			m.c1, m.c2, m.c3, m.c4,
			m.d1, m.d2, m.d3, m.d4
		);


	}
	struct SkeletonNode
	{
		std::string Name;
		XMMATRIX LocalTransform = XMMatrixIdentity();
		int ParentIndex = -1;
		std::vector<int> Children;
	};

	class Skeleton
	{
	public:
		void BuildFromScene(const aiScene* pScene;);
		int GetBoneId(const aiBone* bone);
		void AddBoneOffset(const aiBone* bone);

		bool HasBone(const std::string& name) const;
		int GetBoneIndex(const std::string& name) const;

		void CalculateFinalTransforms();
		void CalculateFinalTransforms(const AnimationClip* animation, float animationTime);

		const std::vector<BoneInfo>& GetBones() const { return m_Bones; }
		std::vector<XMMATRIX> GetFinalMatrices() const;

		XMVECTOR InterpolatePosition(float animationTime, const AnimationChannel* nodeAnim);
		XMVECTOR InterpolateScale(float animationTime, const AnimationChannel* nodeAnim);
		XMVECTOR InterpolateRotation(float animationTime, const AnimationChannel* nodeAnim);
		void ReadNodeHierarchy(float animationTime, const aiNode* node, const XMMATRIX& parentTransform, const AnimationClip* animation, const XMMATRIX& globalInverseTransform, std::unordered_map<std::string, uint32_t>& boneMapping, std::vector<BoneInfo>& boneInfo);
		void EvaluateAnimationAtTime(float timeSeconds, AnimationClip* animation);
		size_t GetBoneCount() const { return m_Bones.size(); }

	private:
		int BuildNodeRecursive(const aiNode* node, int parentIndex);
		void CalculateNodeRecursive(int nodeIndex, const XMMATRIX& parentTransform);
		void CalculateAnimatedNodeRecursive(
			int nodeIndex,
			const XMMATRIX& parentTransform,
			const AnimationClip* animation,
			float animationTime);

	private:
		std::vector<BoneInfo> m_Bones;
		std::unordered_map<std::string, uint32_t> m_BoneNameToIndex;
		const aiScene* m_Scene;
		std::vector<SkeletonNode> m_Nodes;
		std::unordered_map<std::string, int> m_NodeNameToIndex;

		XMMATRIX m_GlobalInverseTransform = XMMatrixIdentity();
	};
}