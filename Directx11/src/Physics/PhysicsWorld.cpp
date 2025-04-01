#include "pch.h"
#include "PhysicsWorld.h"

namespace Engine
{

	

	PhysicsEngine::PhysicsEngine()
		: _body_activation_listener(std::make_unique<MyBodyActivationListener>()),
		_contact_listener(std::make_unique<MyContactListener>())
	{
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
	_physics_system->OptimizeBroadPhase();
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
}
