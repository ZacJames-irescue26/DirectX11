#pragma once
#include <Components\Components.h>
#include "Physics\PhysicsWorld.h"
#include "Entity.h"

namespace Engine
{

	struct LuaRaycastHit
	{
		bool Hit = false;

		std::string Entity;

		XMFLOAT3 Position = {};
		XMFLOAT3 Normal = {};


		float Distance = 0.0f;
	};
	struct LuaOverlapHit
	{
		std::string Entity;

		XMFLOAT3 Position = {};
		XMFLOAT3 Normal = {};

		float Distance = 0.0f;
	};

struct ScriptPhysics
{
	static XMFLOAT3 GetLinearVelocity(Entity* entity)
	{
		if (!entity)
			return XMFLOAT3(0, 0, 0);

		auto* rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return XMFLOAT3(0, 0, 0);

		return PhysicsEngine::FromJoltVector(PhysicsEngine::Get()->GetBodyInterface().GetLinearVelocity(rb->m_BodyID));
	}


	static void SetPosition(Entity* entity, XMFLOAT3 position)
	{
		if (!entity)
			return;

		auto rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;

		PhysicsEngine::Get()->GetBodyInterface().SetPosition(rb->m_BodyID, PhysicsEngine::ToJoltVector(position), JPH::EActivation::Activate);
	}
	static void SetRotationEuler(Entity* entity, const XMFLOAT3& euler)
	{
		if (!entity)
			return;

		auto* rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;

		XMVECTOR q = XMQuaternionRotationRollPitchYaw(
			euler.x,
			euler.y,
			euler.z
		);

		XMFLOAT4 qf;
		XMStoreFloat4(&qf, q);

		PhysicsEngine::Get()->GetBodyInterface().SetRotation(
			rb->m_BodyID,
			JPH::Quat(qf.x, qf.y, qf.z, qf.w),
			JPH::EActivation::Activate
		);
	}
	static void SetRotationQuat(Entity* entity, const XMFLOAT4& quat)
	{
		if (!entity)
			return;

		auto* rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;


		PhysicsEngine::Get()->GetBodyInterface().SetRotation(
			rb->m_BodyID,
			JPH::Quat(quat.x, quat.y, quat.z, quat.w),
			JPH::EActivation::Activate
		);
	}
	static void SetLinearVelocity(Entity* entity, const XMFLOAT3& velocity)
	{
		if (!entity)
			return;

		auto rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;

		PhysicsEngine::Get()->GetBodyInterface().SetLinearVelocity(rb->m_BodyID, PhysicsEngine::ToJoltVector(velocity));
	}

	static void AddForce(Entity* entity, const XMFLOAT3& force)
	{
		if (!entity)
			return;

		auto* rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;

		PhysicsEngine::Get()->GetBodyInterface().AddForce(rb->m_BodyID, PhysicsEngine::ToJoltVector(force));
	}

	static void MoveKinematic(Entity* entity, const XMFLOAT3& position, const XMFLOAT4& rotation, float dt)
	{
		if (!entity)
			return;

		auto* rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;
		PhysicsEngine::Get()->GetBodyInterface().ActivateBody(rb->m_BodyID);
		PhysicsEngine::Get()->GetBodyInterface().MoveKinematic(rb->m_BodyID, PhysicsEngine::ToJoltVector(position), PhysicsEngine::ToJoltQuat(rotation), dt);
	}


	static void AddImpulse(Entity* entity, const XMFLOAT3& impulse)
	{
		
		if (!entity)
			return;

		auto* rb = entity->GetComponent<PhysicsComponent>();

		if (!rb)
			return;

		auto& bodyInterface = PhysicsEngine::Get()->GetBodyInterface();

		bodyInterface.ActivateBody(rb->m_BodyID);

		bodyInterface.AddImpulse(
			rb->m_BodyID,
			PhysicsEngine::ToJoltVector(impulse));

	}

};
}