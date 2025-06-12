#pragma once
#include "Ray.h"
#include "Triangle.h"

namespace Engine
{
	class AABB
	{
	public:
		AABB(XMFLOAT3 min, XMFLOAT3 max)
			: min(XMLoadFloat3(&min)), max(XMLoadFloat3(&max)), minf(min), maxf(max)
		{
			//CreateDebugBox();
		}
		AABB(XMVECTOR min, XMVECTOR max)
			: min(min), max(max)
		{
			XMStoreFloat3(&minf, min);
			XMStoreFloat3(&maxf, max);
			//CreateDebugBox();
		}
		AABB() {}
		//~AABB();
		bool ContainsPoint(XMFLOAT3 point);
		bool ContainsPoint(XMVECTOR point);
		bool intersect(const Ray& ray, float& tNear, float& tFar);
		bool ContainsTriangle(Triangle triangle);
		bool ContainsAABB(AABB aabb);
		bool OverlappingwithSphere(const XMFLOAT3& c, float radius);
		XMVECTOR Center();
		std::array<Engine::AABB*, 8> SplitIntoOct();
		//void CreateDebugBox();

		static AABB& Combine(const AABB& currentBounds, const AABB& addingbounds);
		void extend(const XMFLOAT3& vertex);

		// Helper function to get the centroid of an object
		XMVECTOR getCentroid() const {
			return 0.5f * (min + max);
		}
		XMVECTOR Min()
		{
			return min;
		}
		XMVECTOR Max()
		{
			return max;
		}
		XMFLOAT3 Minf()
		{
			return minf;
		}
		XMFLOAT3 Maxf()
		{
			return maxf;
		}
	protected:


	private:
		XMVECTOR min = {0.0,0.0,0.0,0.0};
		XMVECTOR max = { 0.0,0.0,0.0,0.0 };
		XMFLOAT3 minf = {0.0,0.0,0.0};
		XMFLOAT3 maxf = {0.0,0.0,0.0};

	};

}