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
		void BuildFromScene(const aiScene* pScene);
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
		void ReadNodeHierarchy(float animationTime, int nodeIndex, const XMMATRIX& parentTransform, const AnimationClip* animation);
		void EvaluateAnimationAtTime(float timeSeconds, AnimationClip* animation);
		bool HasNode(const std::string& name) const;
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