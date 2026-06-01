#include "pch.h"
#include "PhysStructs.h"
#include "Jolt/Physics/Body/Body.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "src/Scene/Scene.h"
namespace Engine
{

	
		Engine::BodyFilterJolt::BodyFilterJolt(Scene& scene, const ExcludedEntityMap& excludeMap)
		{
			m_ExcludedBodies.reserve(excludeMap.size());
			for (uint32_t entityID : excludeMap)
			{
				auto ent = scene.GetEntityByID(entityID);
				auto pcomp = ent->GetComponent<PhysicsComponent>();
				m_ExcludedBodies.insert(pcomp->m_BodyID);
				
			}
		}
		bool BodyFilterJolt::ShouldCollide(const JPH::BodyID& inBodyID) const
		{
			return m_ExcludedBodies.find(inBodyID) == m_ExcludedBodies.end();
		}

		bool BodyFilterJolt::ShouldCollideLocked(const JPH::Body& inBody) const
		{
			if (inBody.IsSensor())
			{
				return false;
			}

			return ShouldCollide(inBody.GetID());
		}

	



}