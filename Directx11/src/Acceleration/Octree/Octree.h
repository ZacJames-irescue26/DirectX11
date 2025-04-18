#pragma once
#include "src/illumination/Surfel.h"
#include "src/Math/AABB.h"
#include <array>

namespace Engine
{
	struct OctreeNode
	{
		AABB* boundingbox;

		std::vector<Surfel*> surfels; // Surfels stored in this node

		std::array<Engine::OctreeNode*, 8> children; // Pointers to child nodes

		OctreeNode(AABB* aabb)
			:  boundingbox(aabb)
		{}
		bool IsLeaf() const {
			return children.empty();
		}
		~OctreeNode() {
			for (OctreeNode* child : children) {
				delete child; 
			}
		}
	};

	class Octree
	{
	public:
		// Constructor
		Octree(AABB* sceneBounds, int maxDepth) : maxLevels(maxDepth)
		{
			root = new OctreeNode(sceneBounds);
			Subdivide(root, 0);
		}
	
		~Octree()
		{
			delete root;
		}
		OctreeNode* Root()
		{
			return root;
		}
		void Subdivide(OctreeNode* node, int depth)
		{
			if (depth > maxLevels)
				return;

			auto splitboxes = node->boundingbox->SplitIntoOct();
			auto& refboxes = splitboxes;
			for (int i = 0; i < splitboxes.size(); i++)
			{
				OctreeNode* child = new OctreeNode(refboxes[i]);
				node->children[i] = child;

				Subdivide(child, depth + 1);
			}
		
		}

		//void DebugDraw(Program* shader, const glm::mat4& viewproj, OctreeNode* node = nullptr, int depth = 0)
		//{
		//	if (depth == 0)
		//		node = root;
		//	if (depth > maxLevels)
		//	{
		//		return;
		//	}
		//	if (node != nullptr && node->boundingbox != nullptr)
		//	{
		//		node->boundingbox->DebugDraw(shader, viewproj);
		//	}
		//		for (int j = 0; j < node->children.size(); j++)
		//		{
		//			if (node->children[j] != nullptr) 
		//			{ // Ensure the child exists
		//				DebugDraw(shader, viewproj, node->children[j], depth + 1);
		//			}
		//		}

		//}

		OctreeNode* FindSmallestAABB(OctreeNode* node, const XMFLOAT3& SurfelArea, int depth = 0)
		{
			if (depth > maxLevels)
			{
				return node;
			}
		
			if (!node) return nullptr; // Base case: Node is null.


			// Check if the surfel is within this node's bounding box

			// If this node is a leaf, return its bounding box.
			if (node->IsLeaf()) {
				if (node->boundingbox->ContainsPoint(SurfelArea)) {
					return node;
				}
				else {
					return nullptr; // Surfel not in this leaf.
				}
			}

			// Recursively check children for a smaller enclosing node.
			for (int i = 0; i < node->children.size(); ++i) {
				if (node->children[i] != nullptr && node->children[i]->boundingbox->ContainsPoint(SurfelArea)) {
					OctreeNode* result = FindSmallestAABB(node->children[i], SurfelArea, depth + 1);
					if (result) return result; // Return as soon as we find a valid enclosing node.
				}
			}

			// If no child fully contains the surfel, this node is the smallest enclosing one.
			return node;
		}
	private:
		OctreeNode* root;
		int maxLevels;
	};

}
