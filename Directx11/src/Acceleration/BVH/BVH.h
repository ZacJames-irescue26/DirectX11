#pragma once
#include "Math\AABB.h"
#include <vector>
#include "Graphics/ModelSimple.h"
#include <DirectXMath.h>

//-----------------------------------------------------------------------------
// Mesh-level BVH node
//-----------------------------------------------------------------------------
namespace Engine
{

	struct alignas(16) FlatNode
	{
		XMFLOAT3 minBounds;
		uint32_t IsLeaf = false;
		XMFLOAT3 maxBounds;
		uint32_t    IsBaseNode = false;

		int32_t  leftChild = -1;   // index of left child or -1
		int32_t  rightChild = -1;  // index of right child or -1

		int32_t  triOffset = 0;   // start index in flatTris
		int32_t  triCount = 0;    // number of triangles
	};
	struct MeshBVHNode {
		AABB                    bounds;
		MeshBVHNode* left = nullptr;
		MeshBVHNode* right = nullptr;
		std::vector<Engine::TriangleJustPos> tris; // leaf only
	};
	struct MeshAccel
	{
		MeshAccel(Mesh* mesh, AABB bound, MeshBVHNode* rt)
		: mesh(mesh), bounds(bound), root(rt)
		{ }
		Mesh* mesh;    // pointer to your mesh data
		AABB           bounds;  // overall mesh AABB
		MeshBVHNode* root;    // pointer into your mesh-BVH tree
	};
	struct RootInfo { MeshBVHNode* root; AABB bounds; };
	// the “model” acceleration is just a flat list
	struct ModelAccel
	{
		std::vector<MeshAccel> meshes;
	};
	/*
										Model 
										 |
										 |
									--------------
									|             |
									  TopLayerBVH
										   |
										   |
									----------------
									|       |       |
									mesh0   mesh1   mesh2...
									|       |       |
									|       |       |
									-----MeshBVH-----
									|       |       |
									M0Tris   M1Tris   M2Tris...

	compare AABB with ray to find the current mesh then use the BVH to find triangle
	*/
	class BVHBuilder
	{
		public:
		static std::unique_ptr<ModelAccel> BuildModelAccel(Model model, int leafSize = 20)
		{
			std::unique_ptr <ModelAccel> accel = std::make_unique<ModelAccel>();
			accel->meshes.reserve(model.GetMeshes().size());

			for (int i = 0; i < (int)model.GetMeshes().size(); ++i)
			{
				auto& mesh = model.GetMeshes()[i];
				AABB meshAABB;
				// 1) extract engine triangles
				std::vector<Engine::Triangle> tris;
				tris.reserve(mesh.indices.size() / 3);
				for (size_t j = 0; j < mesh.indices.size(); j += 3)
				{
					XMFLOAT3 v0 = mesh.vertices[mesh.indices[j + 0]].pos;
					XMFLOAT3 v1 = mesh.vertices[mesh.indices[j + 1]].pos;
					XMFLOAT3 v2 = mesh.vertices[mesh.indices[j + 2]].pos;
			
					// Extend the AABB to fit all points and extend if needed
					meshAABB.extend(v0);
					meshAABB.extend(v1);
					meshAABB.extend(v2);
					tris.push_back({
					  v0,v1,v2
						});
				}

				// 2) build per-mesh BVH
				MeshBVHNode* root = BuildMeshBVH(tris, leafSize);

				// 4) store into your accel struct
				accel->meshes.push_back({ &model.GetMeshes()[i], meshAABB, root });
			}

			return accel;
		}

		static MeshBVHNode* BuildMeshBVH(std::vector<Engine::Triangle>& tris, int leafSize)
		{
			auto* node = new MeshBVHNode();

			// 1) Compute this node’s AABB
			node->bounds.Min() = XMVectorSet(+FLT_MAX, +FLT_MAX, +FLT_MAX, 0);
			node->bounds.Max() = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0);
			for (auto& T : tris) {
				XMFLOAT3 p0; XMStoreFloat3(&p0, T.p0);
				XMFLOAT3 p1; XMStoreFloat3(&p1, T.p1);
				XMFLOAT3 p2; XMStoreFloat3(&p2, T.p2);
				node->bounds.extend(p0);
				node->bounds.extend(p1);
				node->bounds.extend(p2);
			}

			// 2) If small enough, make a leaf
			if ((int)tris.size() <= leafSize) {
		
				TriangleJustPos tripos;
				for (const auto tri : tris)
				{
					XMStoreFloat4(&tripos.p0, tri.p0);
					XMStoreFloat4(&tripos.p1, tri.p1); 
					XMStoreFloat4(&tripos.p2, tri.p2);
					node->tris.push_back(tripos) ;
				}
		
				return node;
			}

			// 3) Choose split axis by longest extent
			XMFLOAT3 mn, mx;
			XMStoreFloat3(&mn, node->bounds.Min());
			XMStoreFloat3(&mx, node->bounds.Max());
			XMFLOAT3 extent = { mx.x - mn.x, mx.y - mn.y, mx.z - mn.z };
			int axis = extent.x > extent.y
				? (extent.x > extent.z ? 0 : 2)
				: (extent.y > extent.z ? 1 : 2);

			// 4) Partition triangles by centroid median along that axis
			std::nth_element(
				tris.begin(),
				tris.begin() + tris.size() / 2,
				tris.end(),
				[&]( Triangle& A,  Triangle& B) {
					auto centroid = [&]( Triangle& T) {
				
						XMFLOAT3 p0; XMStoreFloat3(&p0, T.p0);
						XMFLOAT3 p1; XMStoreFloat3(&p1, T.p1);
						XMFLOAT3 p2; XMStoreFloat3(&p2, T.p2);
						return XMFLOAT3(
							(p0.x + p1.x + p2.x) / 3.0f,
							(p0.y + p1.y + p2.y) / 3.0f,
							(p0.z + p1.z + p2.z) / 3.0f
						);
						};
					XMFLOAT3 ca = centroid(A), cb = centroid(B);
					return (&ca.x)[axis] < (&cb.x)[axis];
				}
			);

			// 5) Split the triangle list into two halves
			size_t mid = tris.size() / 2;
			std::vector<Triangle> leftTris(tris.begin(), tris.begin() + mid);
			std::vector<Triangle> rightTris(tris.begin() + mid, tris.end());

			// 6) Recurse
			node->left = BuildMeshBVH(leftTris, leafSize);
			node->right = BuildMeshBVH(rightTris, leafSize);

			return node;
		}

		struct TopLeaf {
			MeshBVHNode* node;
			AABB         bounds;
		};

		//static MeshBVHNode* BuildTopLevelBVH(const ModelAccel& accel) {
		//	// 1) pack each mesh root with its AABB
		//	std::vector<TopLeaf> leaves;
		//	leaves.reserve(accel.meshes.size());
		//	for (auto& m : accel.meshes)
		//		leaves.push_back({ m.root, m.bounds });

		//	// 2) recurse on that single vector
		//	return BuildTopLevelRec(leaves, 0, (int)leaves.size());
		//}

private:
	//	// recursive helper to build the top?level BVH
	//static MeshBVHNode* BuildTopLevelRec(
	//	std::vector<TopLeaf>& leaves,
	//	int start, int end)
	//{
	//	int count = end - start;
	//	if (count == 1)
	//		return leaves[start].node;

	//	// compute enclosing AABB
	//	AABB nodeBounds = leaves[start].bounds;
	//	for (int i = start + 1; i < end; ++i)
	//	{
	//		XMFLOAT3 bound;
	//		XMStoreFloat3(&bound, leaves[i].bounds.Min());
	//		nodeBounds.extend(bound);
	//		XMStoreFloat3(&bound, leaves[i].bounds.Max());
	//		nodeBounds.extend(bound);
	//	}

	//	// choose split axis
	//	XMFLOAT3 mn, mx;
	//	XMStoreFloat3(&mn, nodeBounds.Min());
	//	XMStoreFloat3(&mx, nodeBounds.Max());
	//	XMFLOAT3 ext{ mx.x - mn.x, mx.y - mn.y, mx.z - mn.z };
	//	int axis = ext.x > ext.y
	//		? (ext.x > ext.z ? 0 : 2)
	//		: (ext.y > ext.z ? 1 : 2);

	//	// partition the *single* leaves vector
	//	std::nth_element(
	//		leaves.begin() + start,
	//		leaves.begin() + start + count / 2,
	//		leaves.begin() + end,
	//		[&](auto const& A, auto const& B) {
	//			// compare centroids of A.bounds vs B.bounds
	//			XMFLOAT3 ca, cb;
	//			XMStoreFloat3(&ca, A.bounds.getCentroid());
	//			XMStoreFloat3(&cb, B.bounds.getCentroid());
	//			return (&ca.x)[axis] < (&cb.x)[axis];
	//		}
	//	);
	//	int mid = start + count / 2;

	//	// recurse
	//	MeshBVHNode* left = BuildTopLevelRec(leaves, start, mid);
	//	MeshBVHNode* right = BuildTopLevelRec(leaves, mid, end);

	//	// build internal node
	//	auto* node = new MeshBVHNode();
	//	node->bounds = nodeBounds;
	//	node->left = left;
	//	node->right = right;
	//	return node;
	//}

	public:
		// 2) Flatten now builds that super?root first, then does exactly one recurse()
		static void FlattenMeshBVH(
			ModelAccel* accel,
			std::vector<FlatNode>& outNodes,
			std::vector<Engine::TriangleJustPos>& outTris,
			std::vector<uint32_t>&    baseRoots,
			XMMATRIX model)
		{
			//// build top?level BVH over all meshes
			//MeshBVHNode* top = BuildTopLevelBVH(*accel);

			// now flatten it just once:
			std::function<int(MeshBVHNode*, bool)> recurse = [&](MeshBVHNode* node, bool Ismeshroot)->int {
				int myIndex = (int)outNodes.size();
				outNodes.emplace_back();
				FlatNode& fn = outNodes.back();

				// first transform any leaf?tris into world?space
				std::vector<Engine::TriangleJustPos> worldTriangles;
				worldTriangles.reserve(node->tris.size());
				for (auto& t : node->tris) {
					// if this is a mesh?leaf, node->tris is non?empty
					XMFLOAT4 p0, p1, p2;
					XMStoreFloat4(&p0, XMVector3Transform(XMLoadFloat4(&t.p0), model));
					XMStoreFloat4(&p1, XMVector3Transform(XMLoadFloat4(&t.p1), model));
					XMStoreFloat4(&p2, XMVector3Transform(XMLoadFloat4(&t.p2), model));
					worldTriangles.push_back({ p0,p1,p2 });
				}

				
				XMVECTOR oMin = node->bounds.Min(), oMax = node->bounds.Max();
				XMVECTOR corners[8] = {
				  XMVectorSet(XMVectorGetX(oMin), XMVectorGetY(oMin), XMVectorGetZ(oMin),1),
				  XMVectorSet(XMVectorGetX(oMax), XMVectorGetY(oMin), XMVectorGetZ(oMin),1),
				  XMVectorSet(XMVectorGetX(oMin), XMVectorGetY(oMax), XMVectorGetZ(oMin),1),
				  XMVectorSet(XMVectorGetX(oMax), XMVectorGetY(oMax), XMVectorGetZ(oMin),1),
				  XMVectorSet(XMVectorGetX(oMin), XMVectorGetY(oMin), XMVectorGetZ(oMax),1),
				  XMVectorSet(XMVectorGetX(oMax), XMVectorGetY(oMin), XMVectorGetZ(oMax),1),
				  XMVectorSet(XMVectorGetX(oMin), XMVectorGetY(oMax), XMVectorGetZ(oMax),1),
				  XMVectorSet(XMVectorGetX(oMax), XMVectorGetY(oMax), XMVectorGetZ(oMax),1),
				};
				XMVECTOR wMin = XMVectorSet(+FLT_MAX, +FLT_MAX, +FLT_MAX, 0),
					wMax = XMVectorSet(-FLT_MAX, -FLT_MAX, -FLT_MAX, 0);
				for (int i = 0; i < 8; i++) {
					auto wc = XMVector3TransformCoord(corners[i], model);
					wMin = XMVectorMin(wMin, wc);
					wMax = XMVectorMax(wMax, wc);
				}
				XMStoreFloat3(&fn.minBounds, wMin);
				XMStoreFloat3(&fn.maxBounds, wMax);


				fn.triOffset = 0;
				fn.triCount = 0;

				bool isLeaf = !node->left && !node->right;
				if (isLeaf) {
					fn.triOffset = (int)outTris.size();
					fn.triCount = (int)worldTriangles.size();
					outTris.insert(outTris.end(), worldTriangles.begin(), worldTriangles.end());
					fn.leftChild = -1;
					fn.rightChild = -1;
					fn.IsBaseNode = Ismeshroot;
					fn.IsLeaf = true;
				}
				else {
					fn.IsBaseNode = Ismeshroot;
					if (node->left)  fn.leftChild = recurse(node->left,false);
					if (node->right) fn.rightChild = recurse(node->right,false);
				}
				return myIndex;
			
			};

			for (auto& meshes : accel->meshes)
			{
				uint32_t rootIndex = recurse(meshes.root, true);
				baseRoots.push_back(rootIndex);
			}
		

		};	
	};

}