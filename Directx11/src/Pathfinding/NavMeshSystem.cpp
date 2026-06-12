#include "pch.h"
#include "NavMeshSystem.h"
#include "Recast.h"
#include "DetourNavMeshBuilder.h"
#include "DebugRenderer\DebugRenderer.h"
#include "src/Scene/Scene.h"
namespace Engine
{
	NavMeshSystem::~NavMeshSystem()
	{
		if (m_NavQuery)
		{
			dtFreeNavMeshQuery(m_NavQuery);
			m_NavQuery = nullptr;
		}

		if (m_NavMesh)
		{
			dtFreeNavMesh(m_NavMesh);
			m_NavMesh = nullptr;
		}
	}
	void NavMeshSystem::CollectSceneGeometry(
		Scene& scene,
		std::vector<float>& outVerts,
		std::vector<int>& outTris)
	{
		outVerts.clear();
		outTris.clear();

		for (auto& ent : scene.GetEntities())
		{
			if (!ent)
				continue;
			if (!ent->HasComponent<NavMeshComponent>())
				continue;

			auto* meshComp = ent->GetComponent<StaticMeshComponent>();
			if (!meshComp)
				continue;

			XMMATRIX world = scene.GetWorldMatrix(ent.get());

			Model& model = meshComp->m_Model;

			for (Mesh& mesh : model.GetMeshes())
			{
				int baseVertex =
					static_cast<int>(outVerts.size() / 3);

				for (const Vertex& v : mesh.vertices)
				{
					XMVECTOR p =
						XMLoadFloat3(&v.pos);

					p = XMVector3TransformCoord(p, world);

					XMFLOAT3 wp;
					XMStoreFloat3(&wp, p);

					outVerts.push_back(wp.x);
					outVerts.push_back(wp.y);
					outVerts.push_back(wp.z);
				}

				for (uint32_t index : mesh.indices)
				{
					outTris.push_back(baseVertex + static_cast<int>(index));
				}
			}
		}
	}
	bool NavMeshSystem::BuildFromScene(Scene& scene)
	{
		CollectSceneGeometry(scene, m_Vertices, m_Indices);

		if (m_Vertices.empty() || m_Indices.empty())
			return false;

		const int nverts = static_cast<int>(m_Vertices.size() / 3);
		const int ntris = static_cast<int>(m_Indices.size() / 3);

		rcContext ctx;

		rcConfig cfg = {};
		cfg.cs = 0.3f;
		cfg.ch = 0.2f;
		cfg.walkableSlopeAngle = 45.0f;
		cfg.walkableHeight = static_cast<int>(ceilf(2.0f / cfg.ch));
		cfg.walkableClimb = static_cast<int>(floorf(0.4f / cfg.ch));
		cfg.walkableRadius = static_cast<int>(ceilf(0.5f / cfg.cs));
		cfg.maxEdgeLen = static_cast<int>(12.0f / cfg.cs);
		cfg.maxSimplificationError = 1.3f;
		cfg.minRegionArea = rcSqr(8);
		cfg.mergeRegionArea = rcSqr(20);
		cfg.maxVertsPerPoly = 6;
		cfg.detailSampleDist = 6.0f;
		cfg.detailSampleMaxError = 1.0f;

		rcVcopy(cfg.bmin, m_Vertices.data());
		rcVcopy(cfg.bmax, m_Vertices.data());

		for (int i = 1; i < nverts; ++i)
		{
			const float* v = &m_Vertices[i * 3];

			cfg.bmin[0] = std::min(cfg.bmin[0], v[0]);
			cfg.bmin[1] = std::min(cfg.bmin[1], v[1]);
			cfg.bmin[2] = std::min(cfg.bmin[2], v[2]);

			cfg.bmax[0] = std::max(cfg.bmax[0], v[0]);
			cfg.bmax[1] = std::max(cfg.bmax[1], v[1]);
			cfg.bmax[2] = std::max(cfg.bmax[2], v[2]);
		}

		rcCalcGridSize(cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

		rcHeightfield* solid = rcAllocHeightfield();
		if (!solid)
			return false;

		if (!rcCreateHeightfield(
			&ctx,
			*solid,
			cfg.width,
			cfg.height,
			cfg.bmin,
			cfg.bmax,
			cfg.cs,
			cfg.ch))
		{
			rcFreeHeightField(solid);
			return false;
		}

		std::vector<unsigned char> triAreas(ntris, 0);

		rcMarkWalkableTriangles(
			&ctx,
			cfg.walkableSlopeAngle,
			m_Vertices.data(),
			nverts,
			m_Indices.data(),
			ntris,
			triAreas.data()
		);

		rcRasterizeTriangles(
			&ctx,
			m_Vertices.data(),
			nverts,
			m_Indices.data(),
			triAreas.data(),
			ntris,
			*solid,
			cfg.walkableClimb
		);

		rcFilterLowHangingWalkableObstacles(&ctx, cfg.walkableClimb, *solid);
		rcFilterLedgeSpans(&ctx, cfg.walkableHeight, cfg.walkableClimb, *solid);
		rcFilterWalkableLowHeightSpans(&ctx, cfg.walkableHeight, *solid);

		rcCompactHeightfield* chf = rcAllocCompactHeightfield();
		if (!chf)
		{
			rcFreeHeightField(solid);
			return false;
		}

		if (!rcBuildCompactHeightfield(
			&ctx,
			cfg.walkableHeight,
			cfg.walkableClimb,
			*solid,
			*chf))
		{
			rcFreeCompactHeightfield(chf);
			rcFreeHeightField(solid);
			return false;
		}

		rcFreeHeightField(solid);
		solid = nullptr;

		if (!rcErodeWalkableArea(
			&ctx,
			cfg.walkableRadius,
			*chf))
		{
			rcFreeCompactHeightfield(chf);
			return false;
		}

		if (!rcBuildDistanceField(&ctx, *chf))
		{
			rcFreeCompactHeightfield(chf);
			return false;
		}

		if (!rcBuildRegions(
			&ctx,
			*chf,
			0,
			cfg.minRegionArea,
			cfg.mergeRegionArea))
		{
			rcFreeCompactHeightfield(chf);
			return false;
		}

		rcContourSet* cset = rcAllocContourSet();
		if (!cset)
		{
			rcFreeCompactHeightfield(chf);
			return false;
		}

		if (!rcBuildContours(
			&ctx,
			*chf,
			cfg.maxSimplificationError,
			cfg.maxEdgeLen,
			*cset))
		{
			rcFreeContourSet(cset);
			rcFreeCompactHeightfield(chf);
			return false;
		}

		rcPolyMesh* pmesh = rcAllocPolyMesh();
		if (!pmesh)
		{
			rcFreeContourSet(cset);
			rcFreeCompactHeightfield(chf);
			return false;
		}

		if (!rcBuildPolyMesh(
			&ctx,
			*cset,
			cfg.maxVertsPerPoly,
			*pmesh))
		{
			rcFreePolyMesh(pmesh);
			rcFreeContourSet(cset);
			rcFreeCompactHeightfield(chf);
			return false;
		}

		rcPolyMeshDetail* dmesh = rcAllocPolyMeshDetail();
		if (!dmesh)
		{
			rcFreePolyMesh(pmesh);
			rcFreeContourSet(cset);
			rcFreeCompactHeightfield(chf);
			return false;
		}

		if (!rcBuildPolyMeshDetail(
			&ctx,
			*pmesh,
			*chf,
			cfg.detailSampleDist,
			cfg.detailSampleMaxError,
			*dmesh))
		{
			rcFreePolyMeshDetail(dmesh);
			rcFreePolyMesh(pmesh);
			rcFreeContourSet(cset);
			rcFreeCompactHeightfield(chf);
			return false;
		}
		// Store debug lines before pmesh is freed.
		BuildDebugLinesFromPolyMesh(pmesh, cfg);
		rcFreeCompactHeightfield(chf);
		rcFreeContourSet(cset);

		for (int i = 0; i < pmesh->npolys; ++i)
		{
			pmesh->areas[i] = RC_WALKABLE_AREA;
			pmesh->flags[i] = 1;
		}

		dtNavMeshCreateParams params = {};
		params.verts = pmesh->verts;
		params.vertCount = pmesh->nverts;
		params.polys = pmesh->polys;
		params.polyAreas = pmesh->areas;
		params.polyFlags = pmesh->flags;
		params.polyCount = pmesh->npolys;
		params.nvp = pmesh->nvp;

		params.detailMeshes = dmesh->meshes;
		params.detailVerts = dmesh->verts;
		params.detailVertsCount = dmesh->nverts;
		params.detailTris = dmesh->tris;
		params.detailTriCount = dmesh->ntris;

		params.walkableHeight = 2.0f;
		params.walkableRadius = 0.5f;
		params.walkableClimb = 0.4f;

		rcVcopy(params.bmin, pmesh->bmin);
		rcVcopy(params.bmax, pmesh->bmax);

		params.cs = cfg.cs;
		params.ch = cfg.ch;
		params.buildBvTree = true;

		unsigned char* navData = nullptr;
		int navDataSize = 0;

		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			rcFreePolyMeshDetail(dmesh);
			rcFreePolyMesh(pmesh);
			return false;
		}

		if (m_NavMesh)
			dtFreeNavMesh(m_NavMesh);

		m_NavMesh = dtAllocNavMesh();

		if (!m_NavMesh)
		{
			dtFree(navData);
			rcFreePolyMeshDetail(dmesh);
			rcFreePolyMesh(pmesh);
			return false;
		}

		dtStatus status =
			m_NavMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);

		if (dtStatusFailed(status))
		{
			dtFree(navData);
			rcFreePolyMeshDetail(dmesh);
			rcFreePolyMesh(pmesh);
			return false;
		}

		if (m_NavQuery)
			dtFreeNavMeshQuery(m_NavQuery);

		m_NavQuery = dtAllocNavMeshQuery();

		if (!m_NavQuery)
		{
			rcFreePolyMeshDetail(dmesh);
			rcFreePolyMesh(pmesh);
			return false;
		}

		status = m_NavQuery->init(m_NavMesh, 2048);

		rcFreePolyMeshDetail(dmesh);
		rcFreePolyMesh(pmesh);

		return !dtStatusFailed(status);
	}


	bool NavMeshSystem::FindPath(
		const XMFLOAT3& start,
		const XMFLOAT3& end,
		std::vector<XMFLOAT3>& outPath)
	{
		outPath.clear();

		if (!m_NavQuery || !m_NavMesh)
			return false;

		float startPos[3] = { start.x, start.y, start.z };
		float endPos[3] = { end.x, end.y, end.z };

		float halfExtents[3] = { 2.0f, 4.0f, 2.0f };

		dtQueryFilter filter;
		filter.setIncludeFlags(1);
		filter.setExcludeFlags(0);

		dtPolyRef startRef = 0;
		dtPolyRef endRef = 0;

		float nearestStart[3];
		float nearestEnd[3];

		dtStatus status = m_NavQuery->findNearestPoly(
			startPos,
			halfExtents,
			&filter,
			&startRef,
			nearestStart
		);

		if (dtStatusFailed(status) || startRef == 0)
			return false;

		status = m_NavQuery->findNearestPoly(
			endPos,
			halfExtents,
			&filter,
			&endRef,
			nearestEnd
		);

		if (dtStatusFailed(status) || endRef == 0)
			return false;

		static constexpr int MAX_POLYS = 256;
		dtPolyRef polys[MAX_POLYS];
		int polyCount = 0;

		status = m_NavQuery->findPath(
			startRef,
			endRef,
			nearestStart,
			nearestEnd,
			&filter,
			polys,
			&polyCount,
			MAX_POLYS
		);

		if (dtStatusFailed(status) || polyCount == 0)
			return false;

		static constexpr int MAX_POINTS = 256;
		float straightPath[MAX_POINTS * 3];
		unsigned char straightFlags[MAX_POINTS];
		dtPolyRef straightPolys[MAX_POINTS];
		int straightCount = 0;

		status = m_NavQuery->findStraightPath(
			nearestStart,
			nearestEnd,
			polys,
			polyCount,
			straightPath,
			straightFlags,
			straightPolys,
			&straightCount,
			MAX_POINTS
		);

		if (dtStatusFailed(status))
			return false;

		for (int i = 0; i < straightCount; ++i)
		{
			outPath.push_back({
				straightPath[i * 3 + 0],
				straightPath[i * 3 + 1],
				straightPath[i * 3 + 2]
				});
		}

		return !outPath.empty();
	}


	bool NavMeshSystem::FindRandomPointAround(
		const XMFLOAT3& center,
		float radius,
		XMFLOAT3& outPoint)
	{
		if (!m_NavQuery || !m_NavMesh)
			return false;

		float centerPos[3] =
		{
			center.x,
			center.y,
			center.z
		};

		float halfExtents[3] =
		{
			2.0f,
			4.0f,
			2.0f
		};

		dtQueryFilter filter;
		filter.setIncludeFlags(1);
		filter.setExcludeFlags(0);

		dtPolyRef startRef = 0;
		float nearestCenter[3];

		dtStatus status = m_NavQuery->findNearestPoly(
			centerPos,
			halfExtents,
			&filter,
			&startRef,
			nearestCenter
		);

		if (dtStatusFailed(status) || startRef == 0)
			return false;

		auto random01 = []() -> float
			{
				return static_cast<float>(rand()) /
					static_cast<float>(RAND_MAX);
			};

		dtPolyRef randomRef = 0;
		float randomPoint[3];

		status = m_NavQuery->findRandomPointAroundCircle(
			startRef,
			nearestCenter,
			radius,
			&filter,
			random01,
			&randomRef,
			randomPoint
		);

		if (dtStatusFailed(status) || randomRef == 0)
			return false;

		outPoint = XMFLOAT3(
			randomPoint[0],
			randomPoint[1],
			randomPoint[2]
		);

		return true;
	}


	void NavMeshSystem::BuildDebugLinesFromPolyMesh(
		const rcPolyMesh* pmesh,
		const rcConfig& cfg)
	{
		m_DebugLines.clear();

		if (!pmesh)
			return;

		XMFLOAT3 color = { 0.0f, 1.0f, 0.0f };

		for (int i = 0; i < pmesh->npolys; ++i)
		{
			const unsigned short* p =
				&pmesh->polys[i * 2 * pmesh->nvp];

			for (int j = 0; j < pmesh->nvp; ++j)
			{
				if (p[j] == RC_MESH_NULL_IDX)
					break;

				int next = j + 1;

				if (next >= pmesh->nvp || p[next] == RC_MESH_NULL_IDX)
					next = 0;

				const unsigned short* v0 = &pmesh->verts[p[j] * 3];
				const unsigned short* v1 = &pmesh->verts[p[next] * 3];

				XMFLOAT3 a =
				{
					pmesh->bmin[0] + v0[0] * cfg.cs,
					pmesh->bmin[1] + v0[1] * cfg.ch,
					pmesh->bmin[2] + v0[2] * cfg.cs
				};

				XMFLOAT3 b =
				{
					pmesh->bmin[0] + v1[0] * cfg.cs,
					pmesh->bmin[1] + v1[1] * cfg.ch,
					pmesh->bmin[2] + v1[2] * cfg.cs
				};

				m_DebugLines.push_back({ a, b, color });
			}
		}
	}
	void NavMeshSystem::DebugDraw()
	{
		for (const NavDebugLine& line : m_DebugLines)
		{
			DebugRenderer::Get()->DrawLine(
				line.A,
				line.B,
				line.Color
			);
		}
	}

}