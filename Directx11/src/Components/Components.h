#pragma once

#include "pch.h"
#include "Graphics/ModelSimple.h"
#include <string>
#include "src/Graphics/AnimatedModel.h"
#include "Jolt\Jolt.h"

#include <Jolt/Physics/Body/BodyID.h>               // JPH::BodyID
#include <Jolt/Physics/Body/MotionType.h>            // JPH::EMotionType
#include <Jolt/Physics/Collision/Shape/Shape.h>      // JPH::EShapeSubType
#include "Sol2/sol.hpp"
#include "Utils.h"
#include <fmod.hpp>
#include "src/Sound/Sound.h"
#include "src/Sound/Channel.h"
namespace Engine
{
	enum ComponentEnum
	{
		Transform,
		StaticMesh,
		AnimatedMeshenum,
		PointLight,
		DirectionalLight,
		SpotLight,
		PhysicsComp,
		LuaScript,
		CameraComp,
		AudioComp,
		AudioListenerComp,
		NavMeshComp,
		AgentComp,
		PatrolAgentComp,
		HealthComp
	};

	struct Component
	{
	public:
		virtual ~Component() = default;
		virtual ComponentEnum GetType() const = 0;
		virtual std::unique_ptr<Component> Clone()  = 0;
	};

	struct TransformComponent : public Component
	{

		TransformComponent() {}

		TransformComponent(XMFLOAT3 pos, XMFLOAT3 rot, XMFLOAT3 scale)
			:Position(pos), Rotation(rot), Scale(scale)
		{
			CalculateModelMatrix();
		}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::Transform;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		XMFLOAT3 Position = XMFLOAT3(0.0, 0.0, 0.0);
		XMFLOAT3 Rotation = XMFLOAT3(0.0, 0.0, 0.0);

		// Runtime rotation.
		XMFLOAT4 RotationQuat = XMFLOAT4(0, 0, 0, 1);
		XMFLOAT3 Scale = XMFLOAT3(1.0, 1.0, 1.0);
		XMMATRIX ModelMatrix;

		XMMATRIX CalculateModelMatrix()
		{
			XMVECTOR q = XMLoadFloat4(&RotationQuat);

			float lenSq = XMVectorGetX(XMQuaternionLengthSq(q));

			if (!std::isfinite(lenSq) || lenSq < 0.00001f)
			{
				RotationQuat = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
				q = XMLoadFloat4(&RotationQuat);
			}
			else
			{
				q = XMQuaternionNormalize(q);
				XMStoreFloat4(&RotationQuat, q);
			}

			ModelMatrix =
				XMMatrixScaling(Scale.x, Scale.y, Scale.z) *
				XMMatrixRotationQuaternion(q) *
				XMMatrixTranslation(Position.x, Position.y, Position.z);

			return ModelMatrix;
		}
		std::unique_ptr<Component> Clone()  override
		{
			return std::make_unique<TransformComponent>(*this);
		}


		void SetRotationEulerRadians(const XMFLOAT3& euler)
		{
			Rotation = euler;

			XMVECTOR q = XMQuaternionRotationRollPitchYaw(
				euler.x,
				euler.y,
				euler.z
			);

			XMStoreFloat4(&RotationQuat, XMQuaternionNormalize(q));
			CalculateModelMatrix();
		}
		void SetRotationQuat(const XMFLOAT4& quat)
		{
			XMVECTOR q = XMLoadFloat4(&quat);
			q = XMQuaternionNormalize(q);

			XMStoreFloat4(&RotationQuat, q);

			// Optional, only for editor display.
			Rotation = Utils::QuaternionToEuler(RotationQuat);
		}

		XMFLOAT4 GetRotationQuat() const
		{
			return RotationQuat;
		}
		
		
	};

	struct StaticMeshComponent : public Component
	{
		StaticMeshComponent()
		{ }
		StaticMeshComponent(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader)
		: m_device(device), m_deviceContext(deviceContext), m_cb_vs_vertexshader(&cb_vs_vertexshader)	
		{
			m_filepath = filePath;
			m_Model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader);
			Initialized = true;
		}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::StaticMesh;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
		{
			if (Initialized)
			{
				m_Model.Draw(worldMatrix, viewProjectionMatrix);
			}
		}
		void DrawWithoutCBuffer()
		{
			if (Initialized)
			{
				m_Model.Draw();
			}
		}
		void Initialize(const std::string& path, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>* cb_vs_vertexshader)
		{
			Initialized = m_Model.Initialize(path, device, deviceContext, *cb_vs_vertexshader);
			
		}

		Model m_Model;
		std::string m_filepath;
		bool Initialized = false;

		std::unique_ptr<Component> Clone() override
		{	
			auto clone = std::make_unique<StaticMeshComponent>();

			clone->m_filepath = m_filepath;
			clone->Initialized = false;

			return clone;
		}


	private:
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		ConstantBuffer<CB_VS_vertexShader>* m_cb_vs_vertexshader;
	};
	struct AnimatedMeshComponent : public Component 
	{
		AnimatedMeshComponent(){}
		AnimatedMeshComponent(const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_Anim_VS_vertexShader>& cb_vs_vertexshader)
			: m_device(device), m_deviceContext(deviceContext), m_cb_vs_vertexshader(&cb_vs_vertexshader)
		{
			m_filepath = filePath;
			Initialized = m_Model.Initialize(filePath, device, deviceContext, cb_vs_vertexshader);
		}

		static ComponentEnum StaticType()
		{
			return ComponentEnum::AnimatedMeshenum;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		void Initialize(const std::string& path, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_Anim_VS_vertexShader>* cb_vs_vertexshader)
		{
			Initialized = m_Model.Initialize(path, device, deviceContext, *cb_vs_vertexshader);
		}
		void Draw(const XMMATRIX& worldMatrix, const XMMATRIX& viewProjectionMatrix)
		{
			
			if (Initialized)
			{
				m_Model.Draw(worldMatrix, viewProjectionMatrix);
			}
		}
		void AddAnimation(const std::string& path, std::string name)
		{
			m_AnimPaths.insert({name, path});
			m_Model.LoadAnimationOnly(path, name);
		}
		void SetAnimationindexByName(const std::string& name)
		{
			m_Model.SetAnimationIndexByName(name);
		}
		
		void RemoveAnimation(uint32_t index)
		{
			m_Model.RemoveAnimation(index);
		}
		void RemoveAnimationByName(const std::string& name)
		{
			m_Model.RemoveAnimationByName(name);
		}
		void SetAnimationTime(float time)
		{
			m_Model.SetAnimationTime(time);
		}
		void Update(float deltatime)
		{
			if (m_PlayAnimation)
			{
				m_Model.SetPlayback(m_PlaybackSpeed);
				m_Model.UpdateAnimation(deltatime);
			}
		}
		void SetLooping(bool loop)
		{
			m_Model.SetLooping(loop);
		}
		std::unique_ptr<Component> Clone() override
		{
			auto clone = std::make_unique<AnimatedMeshComponent>();

			clone->m_filepath = m_filepath;
			clone->Initialized = false;
			clone->m_PlayAnimation = m_PlayAnimation;
			clone->m_PlaybackSpeed = m_PlaybackSpeed;
			clone->m_AnimPaths = m_AnimPaths;

			return clone;
		}
		
		
		
		
		
		
		AnimatedModel m_Model;
		std::string currentAnimation;
		std::string m_filepath;
		std::string m_AnimName;
		std::map<std::string, std::string> m_AnimPaths;
		std::string m_AnimPath;
		bool Initialized = false;
		bool m_PlayAnimation;
		float m_PlaybackSpeed= 1.0f;



	private:
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		ConstantBuffer<CB_Anim_VS_vertexShader>* m_cb_vs_vertexshader;

	};

	enum class LightType
	{
		None = 0, Directional = 1, Point = 2, Spot = 3
	};

	struct DirectionalLightComponent : public Component
	{
		DirectionalLightComponent(){}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::DirectionalLight;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}



		XMFLOAT3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		bool CastShadows = true;
		bool SoftShadows = true;
		float LightSize = 0.5f; // For PCSS
		float ShadowAmount = 1.0f;

		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<DirectionalLightComponent>(*this);
		}

	};

	struct PointLightComponent : public Component
	{
		PointLightComponent(){}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::PointLight;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}

		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<PointLightComponent>(*this);
		}

		XMFLOAT3 Radiance = { 1.0f, 1.0f, 1.0f };
		float Intensity = 1.0f;
		float LightSize = 0.5f; // For PCSS
		float MinRadius = 1.f;
		float Radius = 10.f;
		bool CastsShadows = true;
		bool SoftShadows = true;
		float Falloff = 1.f;
	};

	struct SpotLightComponent : public Component
	{
		SpotLightComponent(){}
		static ComponentEnum StaticType()
		{
			return ComponentEnum::SpotLight;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}

		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<SpotLightComponent>(*this);
		}


		XMFLOAT3 Radiance = { 1.0f,1.0,1.0 };
		float Intensity = 1.0f;
		float Range = 10.0f;
		float Angle = 60.0f;
		float AngleAttenuation = 5.0f;
		bool CastsShadows = false;
		bool SoftShadows = false;
		float Falloff = 1.0f;
	};
	struct PhysicsComponent : public Component
	{
		static ComponentEnum StaticType()
		{
			return ComponentEnum::PhysicsComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}

		XMFLOAT3 GetWorldPosition(const TransformComponent& transform) const
		{
			return XMFLOAT3(
				transform.Position.x + ColliderPosition.x,
				transform.Position.y + ColliderPosition.y,
				transform.Position.z + ColliderPosition.z
			);
		}

		std::unique_ptr<Component> Clone() override
		{
			auto clone = std::make_unique<PhysicsComponent>();
			clone->Awake = Awake;
			clone->ColliderPosition = ColliderPosition;
			clone->ColliderSize = ColliderSize;
			clone->ColliderType = ColliderType;
			clone->friction = friction;
			clone->HalfHeight = HalfHeight;
			clone->HalfSize = HalfSize;
			clone->IsSensor = IsSensor;
			clone->radius = radius;
			clone->restitution = restitution;
			clone->RigidBodyType = RigidBodyType;
			return clone;
		}
		JPH::EMotionType RigidBodyType = JPH::EMotionType::Static;
		XMFLOAT3 ColliderPosition = XMFLOAT3(0.0f,0.0f,0.0f);
		XMFLOAT3 ColliderSize = XMFLOAT3(1.0f,1.0f,1.0f);
		JPH::EShapeSubType ColliderType = JPH::EShapeSubType::Box;
		bool Awake = true;
		float radius = 0.0;
		XMFLOAT3 HalfSize = XMFLOAT3(0.0f,0.0f,0.0f);
		float friction = 0.0f;
		float restitution = 0.0f;
		float HalfHeight = 0.0f;
		JPH::BodyID m_BodyID;
		bool IsSensor = false;
	};



	struct LuaScriptComponent : public Component
	{


		std::string ScriptPath = "Scripts/test.lua";

		sol::environment Environment;
		sol::protected_function OnCreate;
		sol::protected_function OnUpdate;

		static ComponentEnum StaticType()
		{
			return ComponentEnum::LuaScript;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			auto clone = std::make_unique<LuaScriptComponent>();

			clone->ScriptPath = ScriptPath;

			// Do not copy Environment, OnCreate, or OnUpdate.
			// Runtime scene will reload the script on OnRuntimeStart.

			return clone;
		}

		void ResetRuntime()
		{
			Environment = sol::environment();
			OnCreate = sol::protected_function();
			OnUpdate = sol::protected_function();
		}
	};


	enum class CameraMode
	{
		Perspective = 0,
		Orthographic = 1
	};

	struct CameraComponent : public Component
	{
		bool Primary = true;

		CameraMode Mode = CameraMode::Perspective;

		float FOVDegrees = 90.0f;
		float NearPlane = 0.1f;
		float FarPlane = 1000.0f;

		float OrthoSize = 10.0f;
		XMFLOAT3 ForwardVector;
		XMFLOAT3 UpVector;
		XMMATRIX ViewMatrix = XMMatrixIdentity();
		XMMATRIX ProjectionMatrix = XMMatrixIdentity();
		XMMATRIX ViewProjectionMatrix = XMMatrixIdentity();

		static ComponentEnum StaticType()
		{
			return ComponentEnum::CameraComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}

		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<CameraComponent>(*this);
		}
	};
	struct AudioComponent : public Component
	{

		static ComponentEnum StaticType()
		{
			return ComponentEnum::AudioComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<AudioComponent>(*this);
		}

		FMOD::Sound** GetSoundAddress()
		{
			return m_Sound.GetSoundAddress();
		}
		FMOD::Sound* GetSound()
		{
			return m_Sound.Get();
		}
		FMOD::Channel** GetChannelAddress()
		{
			return m_Channel.GetChannelAddress();
		}
		FMOD::Channel* GetChannel()
		{
			return m_Channel.Get();
		}
		void SetLoopMode(FMOD_MODE mode)
		{
			loopMode = mode;
			if (m_Sound.Get())
			{
				m_Sound.Get()->setMode(mode);
			}
		}
		void SetLoopMode()
		{
			if (m_Sound.Get())
			{
				m_Sound.Get()->setMode(loopMode);
			}
		}

		void SetMinMaxDistance()
		{
			m_Sound.SetMinMaxDistance(MinDistance, MaxDistance);
		}

		Sound m_Sound;
		Channel m_Channel;
		std::string AudioPath;
		std::vector<std::string> QueueLoadingSounds;	
		float MinDistance = 0.5;
		float MaxDistance = 5000;
		FMOD_MODE loopMode;
		
	


	};
	struct AudioListenerComponent : public Component
	{
		static ComponentEnum StaticType()
		{
			return ComponentEnum::AudioListenerComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<AudioListenerComponent>(*this);
		}

		bool IsListening = true;

	};

	struct NavMeshComponent : public Component
	{
		static ComponentEnum StaticType()
		{
			return ComponentEnum::NavMeshComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<NavMeshComponent>(*this);
		}
	};
	struct NavAgentComponent : public Component
	{
		static ComponentEnum StaticType()
		{
			return ComponentEnum::AgentComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<NavAgentComponent>(*this);
		}
	};


	struct PatrolAgentComponent : public Component
	{
		static ComponentEnum StaticType()
		{
			return ComponentEnum::PatrolAgentComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<PatrolAgentComponent>(*this);
		}

		float PatrolRadius = 10.0f;
		float Speed = 3.0f;
		float StoppingDistance = 1.0f;
		XMFLOAT3 Center = {0.0,0.0,0.0};
		float WaitTime = 1.0f;
		float WaitTimer = 0.0f;

		bool HasPath = false;
		int CurrentPathIndex = 0;

		std::vector<XMFLOAT3> Path;
	};
	struct HealthComponent : public Component
	{
		static ComponentEnum StaticType()
		{
			return ComponentEnum::HealthComp;
		}

		ComponentEnum GetType() const override
		{
			return StaticType();
		}
		std::unique_ptr<Component> Clone() override
		{
			return std::make_unique<HealthComponent>(*this);
		}
		float Health = 100.0f;
		float MaxHealth = 100.0f;
	};

	struct DamageDealerComponent
	{
		float Damage = 10.0f;
		uint64_t OwnerUUID = 0;
	};

	struct ProjectileComponent
	{
		uint64_t OwnerUUID = 0;

		XMFLOAT3 Velocity = { 0, 0, 0 };

		float Damage = 10.0f;
		float Lifetime = 5.0f;
		float Age = 0.0f;
	};
}