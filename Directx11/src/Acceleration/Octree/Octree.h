#pragma once
#include "src/illumination/Surfel.h"
#include "src/Math/AABB.h"
#include <array>
#include <vector>
#include <cstdint>

namespace Engine
{
    struct OctreeNode
    {
        AABB boundingbox;

        std::vector<uint32_t> surfelIndices;

        std::array<OctreeNode*, 8> children;

        OctreeNode(const AABB& aabb)
            : boundingbox(aabb)
        {
            children.fill(nullptr);
        }

        bool IsLeaf() const
        {
            for (OctreeNode* child : children)
            {
                if (child != nullptr)
                    return false;
            }

            return true;
        }

        ~OctreeNode()
        {
            for (OctreeNode* child : children)
            {
                delete child;
            }
        }
    };


	class Octree
	{
	public:
		Octree(const AABB& sceneBounds, int maxDepth)
			: maxLevels(maxDepth)
		{
			root = new OctreeNode(sceneBounds);
			Subdivide(root, 0);
		}

		~Octree()
		{
			delete root;
		}

		void Insert(uint32_t surfelIndex, const XMFLOAT3& position)
		{
			OctreeNode* leaf = FindLeaf(root, position);

			if (!leaf)
				return;

			leaf->surfelIndices.push_back(surfelIndex);
		}

		void QueryRadius(
			const XMFLOAT3& center,
			float radius,
			std::vector<uint32_t>& outIndices) 
		{
			QueryRadiusRecursive(root, center, radius, outIndices);
		}

		OctreeNode* Root()
		{
			return root;
		}

	private:
		OctreeNode* root = nullptr;
		int maxLevels = 0;

		void Subdivide(OctreeNode* node, int depth)
		{
			if (!node)
				return;

			if (depth >= maxLevels)
				return;

			auto splitBoxes = node->boundingbox.SplitIntoOct();

			for (int i = 0; i < 8; ++i)
			{
				// This assumes SplitIntoOct returns AABB objects.
				// If it returns AABB*, dereference/copy carefully.
				node->children[i] = new OctreeNode(splitBoxes[i]);

				Subdivide(node->children[i], depth + 1);
			}
		}

		OctreeNode* FindLeaf(
			OctreeNode* node,
			const XMFLOAT3& position)
		{
			if (!node)
				return nullptr;

			if (!node->boundingbox.ContainsPoint(position))
				return nullptr;

			if (node->IsLeaf())
				return node;

			for (OctreeNode* child : node->children)
			{
				if (child && child->boundingbox.ContainsPoint(position))
					return FindLeaf(child, position);
			}

			return node;
		}

		void QueryRadiusRecursive(
			OctreeNode* node,
			const XMFLOAT3& center,
			float radius,
			std::vector<uint32_t>& outIndices) 
		{
			if (!node)
				return;

			if (!AABBIntersectsSphere(node->boundingbox, center, radius))
				return;

			if (node->IsLeaf())
			{
				outIndices.insert(
					outIndices.end(),
					node->surfelIndices.begin(),
					node->surfelIndices.end()
				);

				return;
			}

			for (OctreeNode* child : node->children)
			{
				QueryRadiusRecursive(child, center, radius, outIndices);
			}
		}

		inline bool AABBIntersectsSphere(
			
			AABB& box,
			const XMFLOAT3& center,
			float radius)
		{
			float sqDist = 0.0f;

			auto testAxis = [&](float v, float minV, float maxV)
				{
					if (v < minV)
					{
						float d = minV - v;
						sqDist += d * d;
					}
					else if (v > maxV)
					{
						float d = v - maxV;
						sqDist += d * d;
					}
				};

			testAxis(center.x, box.Minf().x, box.Maxf().x);
			testAxis(center.y, box.Minf().y, box.Maxf().y);
			testAxis(center.z, box.Minf().z, box.Maxf().z);

			return sqDist <= radius * radius;
		}


	};

}
