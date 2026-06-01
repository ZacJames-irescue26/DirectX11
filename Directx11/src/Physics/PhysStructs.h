#pragma once
#include <unordered_set>
#include <Jolt/Physics/Body/BodyID.h>
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Physics/Body/BodyFilter.h"
namespace JPH {
	class Shape;
	class Body;
	class BodyFilter;
}
namespace Engine
{
	class Scene;
	using ExcludedEntityMap = std::unordered_set<uint32_t>;
	class BodyFilterJolt : public JPH::BodyFilter
	{
	public:

		BodyFilterJolt(Scene& scene, const ExcludedEntityMap& excludeMap);
		bool ShouldCollide(const JPH::BodyID& inBodyID) const;
		bool ShouldCollideLocked(const JPH::Body& inBody) const;
	private:
		std::unordered_set<JPH::BodyID> m_ExcludedBodies;
	};

	class ShapeJolt : public JPH::Shape
	{

	};

}