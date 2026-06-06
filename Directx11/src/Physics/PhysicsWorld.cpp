#include "pch.h"
#include "PhysicsWorld.h"
#include "Scene\Scene.h"
#include "PhysMat.h"
#include "Jolt\Physics\Collision\Shape\CapsuleShape.h"
#include <DirectXMath.h>
#include "Jolt\Physics\Collision\RayCast.h"
#include "Jolt\Physics\Collision\CastResult.h"
#include "PhysStructs.h"
#include "Jolt\Physics\Collision\CollisionCollectorImpl.h"
#include "Jolt\Physics\Collision\CollideShape.h"
#include "Jolt\Physics\Body\BodyLockMulti.h"
#include "RaytraceInfo.h"
namespace Engine
{

	

	PhysicsEngine::PhysicsEngine()
		: _body_activation_listener(std::make_unique<MyBodyActivationListener>()),
		_contact_listener(std::make_unique<MyContactListener>())
	{
		initialise();
	}

	void PhysicsEngine::initialise()
	
	{
		// Register allocation hook. In this example we'll just let Jolt use malloc /
		// free but you can override these if you want (see Memory.h). This needs to
		// be done before any other Jolt function is called.
		JPH::RegisterDefaultAllocator();

		// Install trace and assert callbacks
		JPH::Trace = TraceImpl;
		JPH_IF_ENABLE_ASSERTS(AssertFailed = ErrorLogger::AssertFailedImpl;)

			// Create a factory, this class is responsible for creating instances of
			// classes based on their name or hash and is mainly used for deserialization
			// of saved data. It is not directly used in this example but still required.
			JPH::Factory::sInstance = new JPH::Factory();

		// Register all physics types with the factory and install their collision
		// handlers with the CollisionDispatch class. If you have your own custom
		// shape types you probably need to register their handlers with the
		// CollisionDispatch before calling this function. If you implement your own
		// default material (PhysicsMaterial::sDefault) make sure to initialize it
		// before this function or else this function will create one for you.
		JPH::RegisterTypes();

		// We need a temp allocator for temporary allocations during the physics
		// update. We're pre-allocating 10 MB to avoid having to do allocations during
		// the physics update. B.t.w. 10 MB is way too much for this example but it is
		// a typical value you can use. If you don't want to pre-allocate you can also
		// use TempAllocatorMalloc to fall back to malloc / free.
		_temp_allocator =
			std::make_unique<JPH::TempAllocatorImpl>(10 * 1'024 * 1'024);

		// We need a job system that will execute physics jobs on multiple threads.
		// Typically you would implement the JobSystem interface yourself and let Jolt
		// Physics run on top of your own job scheduler. JobSystemThreadPool is an
		// example implementation.
		_job_system = std::make_unique<JPH::JobSystemThreadPool>(
			JPH::cMaxPhysicsJobs,
			JPH::cMaxPhysicsBarriers,
			static_cast<int>(std::thread::hardware_concurrency()) - 1);

		// This is the max amount of rigid bodies that you can add to the physics
		// system. If you try to add more you'll get an error. Note: This value is low
		// because this is a simple test. For a real project use something in the
		// order of 65536.
		constexpr JPH::uint cMaxBodies = 1'024;

		// This determines how many mutexes to allocate to protect rigid bodies from
		// concurrent access. Set it to 0 for the default settings.
		constexpr JPH::uint cNumBodyMutexes = 0;

		// This is the max amount of body pairs that can be queued at any time (the
		// broad phase will detect overlapping body pairs based on their bounding
		// boxes and will insert them into a queue for the narrowphase). If you make
		// this buffer too small the queue will fill up and the broad phase jobs will
		// start to do narrow phase work. This is slightly less efficient. Note: This
		// value is low because this is a simple test. For a real project use
		// something in the order of 65536.
		constexpr JPH::uint cMaxBodyPairs = 1'024;

		// This is the maximum size of the contact constraint buffer. If more contacts
		// (collisions between bodies) are detected than this number then these
		// contacts will be ignored and bodies will start interpenetrating / fall
		// through the world. Note: This value is low because this is a simple test.
		// For a real project use something in the order of 10240.
		constexpr JPH::uint cMaxContactConstraints = 1'024;

		// Create PhysicsEngineping table from object layer to broadphase layer
		// Note: As this is an interface, PhysicsSystem will take a reference to this
		// so this instance needs to stay alive!
		_broad_phase_layer_interface = std::make_unique<BPLayerInterfaceImpl>();

		// Create class that filters object vs broadphase layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this
		// so this instance needs to stay alive!
		_object_vs_broadphase_layer_filter =
			std::make_unique<ObjectVsBroadPhaseLayerFilterImpl>();

		// Create class that filters object vs object layers
		// Note: As this is an interface, PhysicsSystem will take a reference to this
		// so this instance needs to stay alive!
		_object_vs_object_layer_filter =
			std::make_unique<ObjectLayerPairFilterImpl>();

		// Now we can create the actual physics system.
		_physics_system = std::make_unique<JPH::PhysicsSystem>();
		_physics_system->Init(cMaxBodies,
			cNumBodyMutexes,
			cMaxBodyPairs,
			cMaxContactConstraints,
			*_broad_phase_layer_interface,
			*_object_vs_broadphase_layer_filter,
			*_object_vs_object_layer_filter);

		// A body activation listener gets notified when bodies activate and go to
		// sleep Note that this is called from a job so whatever you do here needs to
		// be thread safe. Registering one is entirely optional.
		_physics_system->SetBodyActivationListener(_body_activation_listener.get());

		// A contact listener gets notified when bodies (are about to) collide, and
		// when they separate again. Note that this is called from a job so whatever
		// you do here needs to be thread safe. Registering one is entirely optional.
		_physics_system->SetContactListener(_contact_listener.get());

		// The main way to interact with the bodies in the physics system is through
		// the body interface. There is a locking and a non-locking variant of this.
		// We're going to use the locking version (even though we're not planning to
		// access bodies from multiple threads)
		//JPH::BodyInterface &body_interface = _physics_system->GetBodyInterface();
		
		
	}

PhysicsEngine::~PhysicsEngine()
{
	UnRegister();
	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

BodyID PhysicsEngine::CreateAndAddObject(BodyCreationSettings settings, EActivation mode)
{
	return _physics_system->GetBodyInterface().CreateAndAddBody(settings, mode);
}

void PhysicsEngine::Optimize()
{
	if (_physics_system)
	{
		_physics_system->OptimizeBroadPhase();
	}
	else
	{
		ErrorLogger::Log("Physic system null");
	}
}

void PhysicsEngine::Update()
{
	// If you take larger steps than 1 / 60th of a second you need to do multiple collision steps in order to keep the simulation stable. Do 1 collision step per 1 / 60th of a second (round up).
	const int cCollisionSteps = 1;

	// Step the world
	_physics_system->Update(cDeltaTime, cCollisionSteps, 1, _temp_allocator.get(), _job_system.get());
}

void PhysicsEngine::RemoveAndDestroyObject(BodyID id)
{
	_physics_system->GetBodyInterface().RemoveBody(id);

	// Destroy the sphere. After this the sphere ID is no longer valid.
	_physics_system->GetBodyInterface().DestroyBody(id);
}

void PhysicsEngine::UnRegister()
{
	UnregisterTypes();
}

RVec3 PhysicsEngine::GetPosition(BodyID id)
{
	return _physics_system->GetBodyInterface().GetPosition(id);

}
void PhysicsEngine::SetPosition(BodyID id, RVec3 position)
{
	_physics_system->GetBodyInterface().SetPosition(id, position, EActivation::DontActivate);

}
void PhysicsEngine::GetPosAndRot(JPH::BodyID id,XMFLOAT3* pos, XMFLOAT4* rot)
{
	JPH::Vec3 jphpos;
	JPH::Quat jphrot;

	_physics_system->GetBodyInterface().GetPositionAndRotation(
		id,
		jphpos,
		jphrot
	);

	if (pos)
	{
		*pos = XMFLOAT3(
			jphpos.GetX(),
			jphpos.GetY(),
			jphpos.GetZ()
		);
	}

	if (rot)
	{
		*rot =XMFLOAT4(
			jphrot.GetX(),
			jphrot.GetY(),
			jphrot.GetZ(),
			jphrot.GetW()
		);
	}
}
void PhysicsEngine::Stop()
{
	BodyIDVector vec;
	_physics_system->GetBodies(vec);
	_physics_system->GetBodyInterface().RemoveBodies(vec.data(), vec.size());
}

void PhysicsEngine::SetGravity(float grav)
{
	_physics_system->SetGravity({ 0.0,-grav,0.0 });
}
void PhysicsEngine::ApplyForce(JPH::BodyID id, XMFLOAT3 force)
{
	JPH::BodyLockWrite bodyLock(_physics_system->GetBodyLockInterface(), id);
	auto& body = bodyLock.GetBody();
	body.AddForce({ force.x,force.y,force.z });
}
void PhysicsEngine::SetSensor(JPH::BodyID id, bool issensor)
{
	JPH::BodyLockWrite bodyLock(_physics_system->GetBodyLockInterface(), id);
	auto& body = bodyLock.GetBody();
	body.SetIsSensor(issensor);
}

void PhysicsEngine::CreateColliders(Scene* scene)
{


	for (auto& entity : scene->GetEntities())
	{
		if (entity->HasComponent<PhysicsComponent>())
		{
			auto PGO = entity->GetComponent<PhysicsComponent>();
			auto transformcomp = entity->GetComponent<TransformComponent>();
			BodyCreationSettings settings;
			settings.mUserData = entity->GetUUID();
			switch (PGO->ColliderType)
			{
			case EShapeSubType::Capsule:
			{

				const auto mat = PhysMat::Create(PGO->friction, PGO->restitution);
				const CapsuleShape* capsule = new CapsuleShape(PGO->HalfHeight, PGO->radius, mat);
				XMFLOAT3 colliderWorldPos = {0.0,0.0,0.0};
				if (transformcomp && PGO)
				{
					XMFLOAT3 colliderWorldPos = PGO->GetWorldPosition(*transformcomp);
				}
				if (PGO->RigidBodyType == EMotionType::Static)
					settings = BodyCreationSettings(capsule, { colliderWorldPos.x, colliderWorldPos.y, colliderWorldPos.z }, Quat::sIdentity(), PGO->RigidBodyType, Layers::NON_MOVING);
				else
					settings = BodyCreationSettings(capsule, { colliderWorldPos.x, colliderWorldPos.y,colliderWorldPos.z }, Quat::sIdentity(), PGO->RigidBodyType, Layers::MOVING);
				break;
			}
			case EShapeSubType::Box:
			{
				const auto mat = PhysMat::Create(PGO->friction, PGO->restitution);
				const BoxShape* box = new BoxShape({ PGO->HalfSize.x, PGO->HalfSize.y, PGO->HalfSize.z });
				
				
				
				XMFLOAT3 colliderWorldPos = {0.0,0.0,0.0};
				if (transformcomp && PGO)
				{
					XMFLOAT3 colliderWorldPos = PGO->GetWorldPosition(*transformcomp);
				}

				if (PGO->RigidBodyType == EMotionType::Static)
					settings = BodyCreationSettings(box, { colliderWorldPos.x, colliderWorldPos.y, colliderWorldPos.z }, Quat::sIdentity(), PGO->RigidBodyType, Layers::NON_MOVING);
				else
					settings = BodyCreationSettings(box, { colliderWorldPos.x, colliderWorldPos.y, colliderWorldPos.z }, Quat::sIdentity(), PGO->RigidBodyType, Layers::MOVING);
				break;
			}
			}
			PGO->m_BodyID = CreateAndAddObject(settings, EActivation::Activate);
			SetSensor(PGO->m_BodyID, PGO->IsSensor);
		}
		
	}

	Optimize();
}

bool PhysicsEngine::CastRay(const RayCastInfo* info, RayHit& outHit, Scene* scene)

{
	outHit.Clear();

	JPH::RayCast ray;
	ray.mOrigin = JPH::Vec3(info->Origin.x, info->Origin.y, info->Origin.z);
	ray.mDirection = ToJoltVector(XMVector3Normalize(XMLoadFloat3(&info->Direction))) * info->MaxDistance;

	JPH::RayCastResult hit;
	JPH::RayCastSettings settings; // defaults are fine
	Engine::BodyFilterJolt bodyFilter(
		*scene,
		info->ExcludedEntities
	);
	// BodyFilter is your custom filter; broadphase/object layer filters left empty
	if (!_physics_system->GetNarrowPhaseQuery().CastRay(JPH::RRayCast(ray), hit, {}, {}, bodyFilter))
	{
		return false;
	}
	
	// Lock the body we hit (thread-safe access)
	JPH::BodyLockRead lock(_physics_system->GetBodyLockInterface(), hit.mBodyID);
	if (!lock.Succeeded()) return false;

	const JPH::Body& body = lock.GetBody();
	const JPH::Vec3  hitWS = ray.GetPointOnRay(hit.mFraction);

	outHit.HitEntity = static_cast<uint64_t>(body.GetUserData());
	outHit.Position = FromJoltVector(hitWS);
	outHit.Normal = FromJoltVector(body.GetWorldSpaceSurfaceNormal(hit.mSubShapeID2, hitWS));
	outHit.Distance = hit.mFraction * ray.mDirection.Length();


	if (void* ud = reinterpret_cast<void*>(body.GetShape()->GetUserData()))
		outHit.HitCollider = reinterpret_cast<ShapeJolt*>(ud); // or set an ID/handle instead

	return true;
}

int32_t PhysicsEngine::OverlapShape(Scene* scene, const ShapeOverlapInfo* shapeOverlapInfo, RayHit** outHits)
{
	m_OverlapIDs.clear();

	JPH::Ref<JPH::Shape> shape = nullptr;

	switch (shapeOverlapInfo->GetCastType())
	{
	case ShapeCastType::Box:
	{
		const auto* boxCastInfo = reinterpret_cast<const BoxOverlapInfo*>(shapeOverlapInfo);
		shape = new JPH::BoxShape(ToJoltVector(boxCastInfo->HalfExtent));
		break;
	}
	case ShapeCastType::Sphere:
	{
		const auto* sphereCastInfo = reinterpret_cast<const SphereOverlapInfo*>(shapeOverlapInfo);
		shape = new JPH::SphereShape(sphereCastInfo->Radius);
		break;
	}
	case ShapeCastType::Capsule:
	{
		const auto* capsuleCastInfo = reinterpret_cast<const CapsuleOverlapInfo*>(shapeOverlapInfo);
		shape = new JPH::CapsuleShape(capsuleCastInfo->HalfHeight, capsuleCastInfo->Radius);
		break;
	}
	}

	JPH::Mat44 worldTransform = JPH::Mat44::sTranslation(ToJoltVector(shapeOverlapInfo->Origin));
	JPH::Vec3 shapeScale = JPH::Vec3(1.0f, 1.0f, 1.0f);

	JPH::CollideShapeSettings settings;
	JPH::AllHitCollisionCollector<JPH::CollideShapeCollector> collector;

	Engine::BodyFilterJolt bodyFilter(
		*scene,
		shapeOverlapInfo->ExcludedEntities
	);

	_physics_system->GetNarrowPhaseQuery().CollideShape(shape, shapeScale, worldTransform, settings, JPH::RVec3::sZero(), collector, {}, {}, bodyFilter);

	int numBodies = static_cast<int>(collector.mHits.size());
	m_OverlapIDs.reserve(numBodies);
	// Lock all bodies in collector.mHits
	for (size_t i = 0; i < numBodies; i++)
		m_OverlapIDs[i] = collector.mHits[i].mBodyID2;

	{
		JPH::BodyLockMultiRead bodyLock(_physics_system->GetBodyLockInterface(), m_OverlapIDs.data(), numBodies);
		for (int i = 0; i < numBodies; i++)
		{
			const JPH::Body* body = bodyLock.GetBody(i);

			if (body == nullptr)
				continue;

			XMFLOAT3 hitPosition = FromJoltVector(collector.mHits[i].mContactPointOn2);

			auto& hitInfo = m_OverlapHitBuffer.emplace_back();
			hitInfo.HitEntity = body->GetUserData();
			hitInfo.Position = hitPosition;
			hitInfo.Normal = FromJoltVector(body->GetWorldSpaceSurfaceNormal(collector.mHits[i].mSubShapeID2, ToJoltVector(hitPosition)));
			XMVECTOR hitpos = XMLoadFloat3(&hitPosition);
			XMVECTOR origin = XMLoadFloat3(&shapeOverlapInfo->Origin);

			XMVECTOR diff = XMVectorSubtract(hitpos, origin);
			XMVECTOR distVec = XMVector3Length(diff);

			float distance = XMVectorGetX(distVec);

			hitInfo.Distance = distance;
			hitInfo.HitCollider = reinterpret_cast<ShapeJolt*>(body->GetShape()->GetUserData());
		}
	}

	*outHits = m_OverlapHitBuffer.data();
	return int32_t(m_OverlapHitBuffer.size());
}


void PhysicsEngine::UnregisterEntity(Entity* entity)
{
	auto* rb = entity->GetComponent<PhysicsComponent>();

	if (!rb)
		return;

	JPH::BodyInterface& bodyInterface =
		_physics_system->GetBodyInterface();

	JPH::BodyID bodyID = rb->m_BodyID;

	if (!bodyID.IsInvalid())
	{
		if (bodyInterface.IsAdded(bodyID))
		{
			bodyInterface.RemoveBody(bodyID);
		}

		bodyInterface.DestroyBody(bodyID);
	}

	rb->m_BodyID = JPH::BodyID();
}














}
