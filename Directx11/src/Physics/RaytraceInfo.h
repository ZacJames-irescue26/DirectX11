#pragma once
#include <unordered_set>
#include <DirectXMath.h>
#include "pch.h"

using ExcludedEntityMap = std::unordered_set<uint64_t>;
namespace Engine
{
	class ShapeJolt;

	struct RayCastInfo
	{
		XMFLOAT3 Origin;
		XMFLOAT3 Direction;
		float MaxDistance;
		ExcludedEntityMap ExcludedEntities;
	};

	enum class ShapeCastType { Box, Sphere, Capsule };
	struct ShapeOverlapInfo
	{
		ShapeOverlapInfo(ShapeCastType castType)
			: m_Type(castType)
		{
		}

		XMFLOAT3 Origin = XMFLOAT3(0.0,0.0,0.0);

		ExcludedEntityMap ExcludedEntities;

		ShapeCastType GetCastType() const { return m_Type; }

	private:
		ShapeCastType m_Type;
	};

	struct BoxOverlapInfo : public ShapeOverlapInfo
	{
		BoxOverlapInfo() : ShapeOverlapInfo(ShapeCastType::Box) {}

		XMFLOAT3 HalfExtent = XMFLOAT3(0.0, 0.0, 0.0);
	};

	struct SphereOverlapInfo : public ShapeOverlapInfo
	{
		SphereOverlapInfo() : ShapeOverlapInfo(ShapeCastType::Sphere) {}

		float Radius = 0.0f;
	};

	struct CapsuleOverlapInfo : public ShapeOverlapInfo
	{
		CapsuleOverlapInfo() : ShapeOverlapInfo(ShapeCastType::Capsule) {}

		float HalfHeight = 0.0f;
		float Radius = 0.0f;
	};
	struct RayHit
	{
		uint64_t HitEntity = 0;
		XMFLOAT3 Position = XMFLOAT3(0.0, 0.0, 0.0);
		XMFLOAT3 Normal = XMFLOAT3(0.0, 0.0, 0.0);
		float Distance = 0.0f;
		ShapeJolt* HitCollider = nullptr;
		void Clear();
	};
}