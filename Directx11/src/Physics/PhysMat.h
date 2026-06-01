#pragma once
#include <Jolt/Physics/Collision/PhysicsMaterial.h>
#include "Jolt/Core/StreamIn.h"
#include "Jolt/Core/StreamOut.h"
#include "Jolt/ObjectStream/SerializableObject.h"
#include "Jolt/Core/RTTI.h"


namespace Engine
{

	class PhysMat final : public JPH::PhysicsMaterial {
	public:

		explicit PhysMat(float friction = 0.6f, float restitution = 0.0f)
			: m_Friction(friction), m_Restitution(restitution) {
		}

		// Not pure virtual in base, but helpful for debug
		const char* GetDebugName() const override { return "PhysMat"; }
		JPH::Color  GetDebugColor() const override { return JPH::Color::sGreen; }

		// Accessors you can use in your engine
		float GetFriction() const { return m_Friction; }
		float GetRestitution() const { return m_Restitution; }

		// Create with ref counting
		static JPH::RefConst<PhysMat> Create(float f, float r) { return new PhysMat(f, r); }

		// If you use SERIALIZABLE: save/restore your fields
		void SaveBinaryState(JPH::StreamOut& s) const override {
			JPH::PhysicsMaterial::SaveBinaryState(s);
			s.Write(m_Friction);
			s.Write(m_Restitution);
		}
		void RestoreBinaryState(JPH::StreamIn& s) override {
			JPH::PhysicsMaterial::RestoreBinaryState(s);
			s.Read(m_Friction);
			s.Read(m_Restitution);
		}

	private:
		float m_Friction = 0.6f;
		float m_Restitution = 0.0f;
	};
}