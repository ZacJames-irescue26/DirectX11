#include "pch.h"
#include "Skeleton.h"
#include "AnimationInfo.h"

namespace Engine
{

		void Skeleton::BuildFromScene(const aiScene* scene)
		{
			m_Bones.clear();
			m_BoneNameToIndex.clear();
			m_Nodes.clear();
			m_NodeNameToIndex.clear();
			m_Scene = scene;
			if (!scene || !scene->mRootNode)
				return;

			m_GlobalInverseTransform =
				XMMatrixInverse(nullptr, AiMatrixToXMMATRIX(scene->mRootNode->mTransformation));

			BuildNodeRecursive(scene->mRootNode, -1);

			// Register all bones and offset matrices from meshes.
			for (uint32_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
			{
				aiMesh* mesh = scene->mMeshes[meshIndex];

				for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
				{
					AddBoneOffset(mesh->mBones[boneIndex]);
				}
			}
		}

		int Skeleton::BuildNodeRecursive(const aiNode* node, int parentIndex)
		{
			SkeletonNode outNode;
			outNode.Name = node->mName.C_Str();
			outNode.LocalTransform = AiMatrixToXMMATRIX(node->mTransformation);
			outNode.ParentIndex = parentIndex;

			int nodeIndex = static_cast<int>(m_Nodes.size());

			m_NodeNameToIndex[outNode.Name] = nodeIndex;
			m_Nodes.push_back(outNode);

			if (parentIndex >= 0)
			{
				m_Nodes[parentIndex].Children.push_back(nodeIndex);
			}

			for (uint32_t i = 0; i < node->mNumChildren; ++i)
			{
				BuildNodeRecursive(node->mChildren[i], nodeIndex);
			}

			return nodeIndex;
		}

		int Skeleton::GetBoneId(const aiBone* bone)
		{
			std::string boneName = bone->mName.C_Str();

			auto it = m_BoneNameToIndex.find(boneName);

			if (it != m_BoneNameToIndex.end())
				return static_cast<int>(it->second);

			uint32_t newIndex = static_cast<uint32_t>(m_Bones.size());

			m_BoneNameToIndex[boneName] = newIndex;
			m_Bones.emplace_back(AiMatrixToXMMATRIX(bone->mOffsetMatrix));

			return static_cast<int>(newIndex);
		}

		void Skeleton::AddBoneOffset(const aiBone* bone)
		{
			int boneId = GetBoneId(bone);

			if (boneId >= 0 && boneId < static_cast<int>(m_Bones.size()))
			{
				m_Bones[boneId].OffsetMatrix =
					AiMatrixToXMMATRIX(bone->mOffsetMatrix);
			}
		}

		bool Skeleton::HasBone(const std::string& name) const
		{
			return m_BoneNameToIndex.find(name) != m_BoneNameToIndex.end();
		}

		int Skeleton::GetBoneIndex(const std::string& name) const
		{
			auto it = m_BoneNameToIndex.find(name);

			if (it == m_BoneNameToIndex.end())
				return -1;

			return static_cast<int>(it->second);
		}

		void Skeleton::CalculateFinalTransforms()
		{
			if (m_Nodes.empty())
				return;

			CalculateNodeRecursive(0, XMMatrixIdentity());
		}

		void Skeleton::CalculateNodeRecursive(
			int nodeIndex,
			const XMMATRIX& parentTransform)
		{
			const SkeletonNode& node = m_Nodes[nodeIndex];

			XMMATRIX globalTransform = parentTransform * node.LocalTransform;

			auto boneIt = m_BoneNameToIndex.find(node.Name);

			if (boneIt != m_BoneNameToIndex.end())
			{
				uint32_t boneIndex = boneIt->second;

				m_Bones[boneIndex].FinalTransformation =
					globalTransform * m_Bones[boneIndex].OffsetMatrix;
			}

			for (int childIndex : node.Children)
			{
				CalculateNodeRecursive(childIndex, globalTransform);
			}
		}

		std::vector<XMMATRIX> Skeleton::GetFinalMatrices() const
		{
			std::vector<XMMATRIX> result;
			result.resize(m_Bones.size());

			for (size_t i = 0; i < m_Bones.size(); ++i)
			{
				result[i] = m_Bones[i].FinalTransformation;
			}

			return result;
		}
		const AnimationChannel* FindNodeAnim(
			const AnimationClip* animation,
			const std::string& nodeName)
		{
			if (!animation)
				return nullptr;

			for (size_t i = 0; i < animation->Channels.size(); ++i)
			{
				const AnimationChannel& channel = animation->Channels[i];

				if (nodeName == channel.BoneName)
					return &channel;
			}

			return nullptr;
		}
		
		XMVECTOR Skeleton::InterpolatePosition(float animationTime, const AnimationChannel* nodeAnim)
		{
			if (!nodeAnim || nodeAnim->Positions.empty())
				return XMVectorZero();

			if (nodeAnim->Positions.size() == 1)
			{
				const XMFLOAT3& v = nodeAnim->Positions[0].Position;
				return XMVectorSet(v.x, v.y, v.z, 0.0f);
			}

			uint32_t index = static_cast<uint32_t>(nodeAnim->Positions.size() - 2);

			for (uint32_t i = 0; i < nodeAnim->Positions.size() - 1; ++i)
			{
				if (animationTime < static_cast<float>(nodeAnim->Positions[i + 1].mTime))
				{
					index = i;
					break;
				}
			}

			uint32_t nextIndex = index + 1;

			float t1 = static_cast<float>(nodeAnim->Positions[index].mTime);
			float t2 = static_cast<float>(nodeAnim->Positions[nextIndex].mTime);

			float factor = (animationTime - t1) / std::max(t2 - t1, 1e-6f);
			factor = std::clamp(factor, 0.0f, 1.0f);

			const XMFLOAT3& start = nodeAnim->Positions[index].Position;
			const XMFLOAT3& end = nodeAnim->Positions[nextIndex].Position;

			XMVECTOR a = XMVectorSet(start.x, start.y, start.z, 0.0f);
			XMVECTOR b = XMVectorSet(end.x, end.y, end.z, 0.0f);

			return XMVectorLerp(a, b, factor);
		}

		XMVECTOR Skeleton::InterpolateScale(float animationTime, const AnimationChannel* nodeAnim)
		{
			if (!nodeAnim || nodeAnim->Scales.empty())
				return XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);

			if (nodeAnim->Scales.size() == 1)
			{
				const XMFLOAT3& v = nodeAnim->Scales[0].Scale;
				return XMVectorSet(v.x, v.y, v.z, 0.0f);
			}

			uint32_t index = static_cast<uint32_t>(nodeAnim->Scales.size() - 2);

			for (uint32_t i = 0; i < nodeAnim->Scales.size() - 1; ++i)
			{
				if (animationTime < static_cast<float>(nodeAnim->Scales[i + 1].mTime))
				{
					index = i;
					break;
				}
			}

			uint32_t nextIndex = index + 1;

			float t1 = static_cast<float>(nodeAnim->Scales[index].mTime);
			float t2 = static_cast<float>(nodeAnim->Scales[nextIndex].mTime);

			float factor = (animationTime - t1) / std::max(t2 - t1, 1e-6f);
			factor = std::clamp(factor, 0.0f, 1.0f);

			const XMFLOAT3& start = nodeAnim->Scales[index].Scale;
			const XMFLOAT3& end = nodeAnim->Scales[nextIndex].Scale;

			XMVECTOR a = XMVectorSet(start.x, start.y, start.z, 0.0f);
			XMVECTOR b = XMVectorSet(end.x, end.y, end.z, 0.0f);

			return XMVectorLerp(a, b, factor);
		}

		XMVECTOR Skeleton::InterpolateRotation(float animationTime, const AnimationChannel* nodeAnim)
		{
			if (!nodeAnim || nodeAnim->Rotations.empty())
				return XMQuaternionIdentity();

			if (nodeAnim->Rotations.size() == 1)
			{
				const XMFLOAT4& q = nodeAnim->Rotations[0].Rotation;

				// XMFLOAT4 is assumed to store x, y, z, w
				return XMQuaternionNormalize(
					XMVectorSet(q.x, q.y, q.z, q.w)
				);
			}

			uint32_t index = static_cast<uint32_t>(nodeAnim->Rotations.size() - 2);

			for (uint32_t i = 0; i < nodeAnim->Rotations.size() - 1; ++i)
			{
				if (animationTime < static_cast<float>(nodeAnim->Rotations[i + 1].mTime))
				{
					index = i;
					break;
				}
			}

			uint32_t nextIndex = index + 1;

			float t1 = static_cast<float>(nodeAnim->Rotations[index].mTime);
			float t2 = static_cast<float>(nodeAnim->Rotations[nextIndex].mTime);

			float factor = (animationTime - t1) / std::max(t2 - t1, 1e-6f);
			factor = std::clamp(factor, 0.0f, 1.0f);

			const XMFLOAT4& startQ = nodeAnim->Rotations[index].Rotation;
			const XMFLOAT4& endQ = nodeAnim->Rotations[nextIndex].Rotation;

			XMVECTOR a = XMQuaternionNormalize(
				XMVectorSet(startQ.x, startQ.y, startQ.z, startQ.w)
			);

			XMVECTOR b = XMQuaternionNormalize(
				XMVectorSet(endQ.x, endQ.y, endQ.z, endQ.w)
			);

			return XMQuaternionNormalize(
				XMQuaternionSlerp(a, b, factor)
			);
		}
		bool HasPositionKeys(const AnimationChannel* nodeAnim)
		{
			return nodeAnim && nodeAnim->Positions.size() > 0;
		}

		bool HasRotationKeys(const AnimationChannel* nodeAnim)
		{
			return nodeAnim && nodeAnim->Rotations.size() > 0;
		}

		bool HasScaleKeys(const AnimationChannel* nodeAnim)
		{
			return nodeAnim && nodeAnim->Scales.size() > 0;
		}
		void Skeleton::ReadNodeHierarchy(
			float animationTime,
			const aiNode* node,
			const XMMATRIX& parentTransform,
			const AnimationClip* animation,
			const XMMATRIX& globalInverseTransform,
			std::unordered_map<std::string, uint32_t>& boneMapping,
			std::vector<BoneInfo>& boneInfo)
		{
			std::string nodeName = node->mName.C_Str();

			XMMATRIX nodeTransform = AiMatrixToXMMATRIX(node->mTransformation);

			const AnimationChannel* nodeAnim = FindNodeAnim(animation, nodeName);

			if (nodeAnim)
			{
				XMVECTOR translation = XMVectorZero();
				XMVECTOR rotation = XMQuaternionIdentity();
				XMVECTOR scale = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);

				if (HasPositionKeys(nodeAnim))
					translation = InterpolatePosition(animationTime, nodeAnim);

				if (HasRotationKeys(nodeAnim))
					rotation = InterpolateRotation(animationTime, nodeAnim);

				if (HasScaleKeys(nodeAnim))
					scale = InterpolateScale(animationTime, nodeAnim);

				XMMATRIX T = XMMatrixTranslationFromVector(translation);
				XMMATRIX R = XMMatrixRotationQuaternion(rotation);
				XMMATRIX S = XMMatrixScalingFromVector(scale);

				nodeTransform = S * R * T;
			}

			XMMATRIX globalTransform = parentTransform * nodeTransform;

			auto it = boneMapping.find(nodeName);
			if (it != boneMapping.end())
			{
				uint32_t boneIndex = it->second;
				boneInfo[boneIndex].FinalTransformation =
					globalInverseTransform * globalTransform * boneInfo[boneIndex].OffsetMatrix;
				//boneInfo[boneIndex].FinalTransformation = globalTransform * boneInfo[boneIndex].OffsetMatrix;
			}

			for (uint32_t i = 0; i < node->mNumChildren; ++i)
			{
				ReadNodeHierarchy(
					animationTime,
					node->mChildren[i],
					globalTransform,
					animation,
					globalInverseTransform,
					boneMapping,
					boneInfo);
			}
		}


		void Skeleton::EvaluateAnimationAtTime(float timeSeconds, AnimationClip* animation)
		{
			if (!m_Scene || !animation)
				return;

			float ticksPerSecond = (animation->TicksPerSecond != 0.0)
				? (float)animation->TicksPerSecond
				: 25.0f;

			float timeInTicks = timeSeconds * ticksPerSecond;
			float animationTime = std::fmod(timeInTicks, (float)animation->Duration);
			ReadNodeHierarchy(
				animationTime,
				m_Scene->mRootNode,
				XMMatrixIdentity(),
				animation,
				m_GlobalInverseTransform,
				m_BoneNameToIndex,
				m_Bones);

		}




		

}