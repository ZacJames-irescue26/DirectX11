#pragma once
#include "ErrorLogger.h"
#include "Game/PhysicsObject.h"
// Jolt includes
#include <Jolt/Core/Core.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/IssueReporting.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Math/Quat.h>
#include <Jolt/Math/Real.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Body/MotionType.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/EActivation.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>
namespace Engine
{
// Callback for traces, connect this to your own trace function if you have one
static void TraceImpl(const char* inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Print to the TTY
	OutputDebugString((LPCWSTR)(buffer));
}

namespace Layers
{
	static constexpr JPH::ObjectLayer NON_MOVING{ 0 };
	static constexpr JPH::ObjectLayer MOVING{ 1 };
	static constexpr JPH::ObjectLayer NUM_LAYERS{ 2 };
}; // namespace Layers

/// Class that determines if two object layers can collide
class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
{
public:
	[[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inObject1,
		JPH::ObjectLayer inObject2) const override
	{
		switch (inObject1)
		{
		case Layers::NON_MOVING:
			return inObject2 ==
				Layers::MOVING; // Non moving only collides with moving
		case Layers::MOVING:
			return true; // Moving collides with everything
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// Each broadphase layer results in a separate bounding volume tree in the broad
// phase. You at least want to have a layer for non-moving and moving objects to
// avoid having to update a tree full of static objects every frame. You can
// have a 1-on-1 mapping between object layers and broadphase layers (like in
// this case) but if you have many object layers you'll be creating many broad
// phase trees, which is not efficient. If you want to fine tune your broadphase
// layers define JPH_TRACK_BROADPHASE_STATS and look at the stats reported on
// the TTY.
namespace BroadPhaseLayers
{
	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	static constexpr JPH::BroadPhaseLayer MOVING(1);
	static constexpr JPH::uint NUM_LAYERS(2);
}; // namespace BroadPhaseLayers

// BroadPhaseLayerInterface implementation
// This defines a mapping between object and broadphase layers.
class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
{
public:
	BPLayerInterfaceImpl()
	{
		// Create a mapping table from object to broad phase layer
		mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
		mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
	}

	[[nodiscard]] JPH::uint GetNumBroadPhaseLayers() const override
	{
		return BroadPhaseLayers::NUM_LAYERS;
	}

	[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(
		JPH::ObjectLayer inLayer) const override
	{
		JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
		return mObjectToBroadPhase[inLayer];
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	virtual const char* GetBroadPhaseLayerName(
		JPH::BroadPhaseLayer inLayer) const override
	{
		switch ((JPH::BroadPhaseLayer::Type)inLayer)
		{
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::NON_MOVING:
			return "NON_MOVING";
		case (JPH::BroadPhaseLayer::Type)BroadPhaseLayers::MOVING:
			return "MOVING";
		default:
			JPH_ASSERT(false);
			return "INVALID";
		}
}
#endif // JPH_EXTERNAL_PROFILE || JPH_PROFILE_ENABLED

private:
	JPH::BroadPhaseLayer mObjectToBroadPhase[Layers::NUM_LAYERS];
	//std::array<JPH::BroadPhaseLayer, Layers::NUM_LAYERS> mObjectToBroadPhase;
};

/// Class that determines if an object layer can collide with a broadphase layer
class ObjectVsBroadPhaseLayerFilterImpl
	: public JPH::ObjectVsBroadPhaseLayerFilter
{
public:
	[[nodiscard]] bool ShouldCollide(
		JPH::ObjectLayer inLayer1,
		JPH::BroadPhaseLayer inLayer2) const override
	{
		switch (inLayer1)
		{
		case Layers::NON_MOVING:
			return inLayer2 == BroadPhaseLayers::MOVING;
		case Layers::MOVING:
			return true;
		default:
			JPH_ASSERT(false);
			return false;
		}
	}
};

// An example contact listener
class MyContactListener : public JPH::ContactListener
{
public:
	// See: ContactListener
	JPH::ValidateResult OnContactValidate(
		const JPH::Body& /* inBody1 */,
		const JPH::Body& /* inBody2 */,
		JPH::RVec3Arg /* inBaseOffset */,
		const JPH::CollideShapeResult& /* inCollisionResult */) override
	{

		// Allows you to ignore a contact before it is created (using layers to not
		// make objects collide is cheaper!)
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	void OnContactAdded(const JPH::Body& /* inBody1 */,
		const JPH::Body& /* inBody2 */,
		const JPH::ContactManifold& /* inManifold */,
		JPH::ContactSettings& /* ioSettings */) override
	{
	}

	void OnContactPersisted(const JPH::Body& /* inBody1 */,
		const JPH::Body& /* inBody2 */,
		const JPH::ContactManifold& /* inManifold */,
		JPH::ContactSettings& /* ioSettings */) override
	{
	}

	void OnContactRemoved(
		const JPH::SubShapeIDPair& /* inSubShapePair */) override
	{
	}
};

// An example activation listener
class MyBodyActivationListener : public JPH::BodyActivationListener
{
public:
	void OnBodyActivated(const JPH::BodyID& /* inBodyID */,
		JPH::uint64 /* inBodyUserData */) override
	{
	}

	void OnBodyDeactivated(const JPH::BodyID& /* &inBodyID */,
		JPH::uint64 /* inBodyUserData */) override
	{
	}
};

class PhysicsEngine
{
public:
	PhysicsEngine();
	~PhysicsEngine();
	// mutator methods
	void initialise();
	

	BodyID CreateAndAddObject(BodyCreationSettings settings, EActivation mode);
	void Optimize();
	void Update();
	void RemoveAndDestroyObject(BodyID id);
	void UnRegister();
	RVec3 GetPosition(BodyID id);
	void SetPosition(BodyID id, RVec3 position);
private:
	JPH::uint _step{ 0 };
	std::unique_ptr<JPH::PhysicsSystem> _physics_system;
	std::unique_ptr<JPH::TempAllocatorImpl> _temp_allocator;
	std::unique_ptr<JPH::JobSystemThreadPool> _job_system;
	std::unique_ptr<MyBodyActivationListener> _body_activation_listener;
	std::unique_ptr<MyContactListener> _contact_listener;
	std::unique_ptr<BPLayerInterfaceImpl> _broad_phase_layer_interface;
	std::unique_ptr<ObjectVsBroadPhaseLayerFilterImpl>
		_object_vs_broadphase_layer_filter;
	std::unique_ptr<ObjectLayerPairFilterImpl> _object_vs_object_layer_filter;
	
	float cDeltaTime = 1.0f/60.0f;
};



}
