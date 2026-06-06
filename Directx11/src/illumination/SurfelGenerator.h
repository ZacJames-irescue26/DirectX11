#pragma once
#include "Surfel.h"
#include "../Game/GameObject.h"
#include "src/Acceleration/Octree/Octree.h"
namespace Engine
{
	/*class SurfelGenerator
	{
	public:

		SurfelGenerator(ID3D11Device* device, ID3D11DeviceContext* devicecontext, Octree* octree,  std::vector<GameObject>& Meshes);
		SurfelGenerator(ID3D11Device* device, ID3D11DeviceContext* devicecontext, const std::string& filename);
		~SurfelGenerator();
		void SaveToFile(const std::string& filename);
		bool ReadSurfels(const std::string& filename);
		void GenerateSurfelsOnMesh(ID3D11Device* device, ID3D11DeviceContext* devicecontext, Octree* octree,  std::vector<GameObject>& Meshes);

		std::vector<Surfel*> GeneratedSurfels;
	private:
		
		int openfiletries = 0;
	};*/





	class SurfelBuilder
	{
	public:
		SurfelBuilder(Engine::Octree& octree)
			: m_Octree(octree)
		{
		}

		uint32_t AddOrMerge(const Surfel& candidate)
		{
			m_QueryBuffer.clear();

			m_Octree.QueryRadius(
				candidate.position,
				MergeDistance,
				m_QueryBuffer
			);

			for (uint32_t index : m_QueryBuffer)
			{
				Surfel& existing = m_Surfels[index];

				if (AreSimilar(candidate, existing))
				{
					Merge(existing, candidate);
					return index;
				}
			}

			uint32_t newIndex =
				static_cast<uint32_t>(m_Surfels.size());

			m_Surfels.push_back(candidate);

			m_Octree.Insert(newIndex, candidate.position);

			return newIndex;
		}

		const std::vector<Surfel>& GetSurfels() const
		{
			return m_Surfels;
		}

	private:
		Engine::Octree& m_Octree;

		std::vector<Surfel> m_Surfels;
		std::vector<uint32_t> m_QueryBuffer;

		float MergeDistance = 0.20f;
		float NormalThreshold = 0.90f;
		float AlbedoThreshold = 0.35f;

		bool AreSimilar(const Surfel& a, const Surfel& b) const
		{
			

			XMVECTOR pa = XMLoadFloat3(&a.position);
			XMVECTOR pb = XMLoadFloat3(&b.position);

			float distSq =
				XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(pa, pb)));

			if (distSq > MergeDistance * MergeDistance)
				return false;

			XMVECTOR na = XMVector3Normalize(XMLoadFloat3(&a.normal));
			XMVECTOR nb = XMVector3Normalize(XMLoadFloat3(&b.normal));

			float ndot = XMVectorGetX(XMVector3Dot(na, nb));

			if (ndot < NormalThreshold)
				return false;

			XMVECTOR aa = XMLoadFloat4(&a.albedo);
			XMVECTOR ab = XMLoadFloat4(&b.albedo);

			float colorDist =
				XMVectorGetX(XMVector3Length(XMVectorSubtract(aa, ab)));

			if (colorDist > AlbedoThreshold)
				return false;

			return true;
		}

		void Merge(Surfel& dst, const Surfel& src)
		{

			float dstArea = std::max(dst.radius, 0.0001f);
			float srcArea = std::max(src.radius, 0.0001f);
			float totalArea = dstArea + srcArea;

			XMVECTOR dp = XMLoadFloat3(&dst.position);
			XMVECTOR sp = XMLoadFloat3(&src.position);

			XMVECTOR dn = XMLoadFloat3(&dst.normal);
			XMVECTOR sn = XMLoadFloat3(&src.normal);

			XMVECTOR da = XMLoadFloat4(&dst.albedo);
			XMVECTOR sa = XMLoadFloat4(&src.albedo);

			XMVECTOR mergedP =
				(dp * dstArea + sp * srcArea) / totalArea;

			XMVECTOR mergedN =
				XMVector3Normalize(dn * dstArea + sn * srcArea);

			XMVECTOR mergedA =
				(da * dstArea + sa * srcArea) / totalArea;

			XMStoreFloat3(&dst.position, mergedP);
			XMStoreFloat3(&dst.normal, mergedN);
			XMStoreFloat4(&dst.albedo, mergedA);

			dst.radius = totalArea;
		}
	};


}


