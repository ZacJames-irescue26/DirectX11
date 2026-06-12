#pragma once

#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"
#include "Recast.h"

namespace Engine
{
	struct NavDebugLine
	{
		XMFLOAT3 A;
		XMFLOAT3 B;
		XMFLOAT3 Color;
	};
	class Scene;
	class NavMeshSystem
	{
	public:
		NavMeshSystem() = default;
		~NavMeshSystem();
		void CollectSceneGeometry(Scene& scene, std::vector<float>& outVerts, std::vector<int>& outTris);
		bool BuildFromScene(Scene& scene);

		bool FindPath(
			const XMFLOAT3& start,
			const XMFLOAT3& end,
			std::vector<XMFLOAT3>& outPath);

		bool FindRandomPointAround(const XMFLOAT3& center, float radius, XMFLOAT3& outPoint);
		void BuildDebugLinesFromPolyMesh(const rcPolyMesh* pmesh, const rcConfig& cfg);
		void DebugDraw();
	private:
		

		std::vector<NavDebugLine> m_DebugLines;
		dtNavMesh* m_NavMesh = nullptr;
		dtNavMeshQuery* m_NavQuery = nullptr;

		std::vector<float> m_Vertices; // xyz xyz xyz
		std::vector<int> m_Indices;    // triangle indices
	};
}