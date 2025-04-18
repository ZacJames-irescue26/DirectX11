#pragma once
#include "../Math/AABB.h"


namespace Engine
{
	class Surfel
	{
	public:

		Surfel(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT4 albedo, float rad);
		Surfel(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT4 albedo);
		Surfel() {}
		AABB aabb;
		XMFLOAT3 position;
		XMFLOAT3 normal;
		XMFLOAT4 albedo;
		XMFLOAT3 diffuse;
		XMFLOAT3 indirectRadiance;
		AABB ComputeSurfelBounds(const XMFLOAT3& surfelPosition, float surfelRadius);
		float radius = 0.01;


	private:
	};

}
