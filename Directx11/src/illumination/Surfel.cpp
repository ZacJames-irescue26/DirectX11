#include "pch.h"
#include "Surfel.h"
namespace Engine
{
	Surfel::Surfel(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT4 albedo, float rad)
		: position(pos), normal(norm), albedo(albedo), radius(rad)

	{
		aabb = ComputeSurfelBounds(position, rad);
	}

	Surfel::Surfel(XMFLOAT3 pos, XMFLOAT3 norm, XMFLOAT4 albedo)
		: position(pos), normal(norm), albedo(albedo), radius(1.0)
	{
		aabb = ComputeSurfelBounds(position, radius);
	}

	AABB Surfel::ComputeSurfelBounds(const XMFLOAT3& surfelPosition, float surfelRadius) {
		XMVECTOR min = XMLoadFloat3(&surfelPosition) - XMVectorReplicate(surfelRadius);
		XMVECTOR max = XMLoadFloat3(&surfelPosition)+ XMVectorReplicate(surfelRadius);
		return AABB{ min, max };
	}

}