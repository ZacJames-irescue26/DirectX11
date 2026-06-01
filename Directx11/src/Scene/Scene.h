#pragma once 
#include "Entity.h"
#include "UUID.h"
#include "src/Physics/PhysicsWorld.h"
#include "Scripting/ScriptEngine.h"
#include "Keyboard/KeyboardClass.h"
#include "Mouse/MouseClass.h"
#include "src/Graphics/Graphics.h"

namespace Engine { class PhysicsEngine; }

namespace Engine
{

	class Scene
	{
	public:
		void Stop();
		void Play();
		void PlayUpdate(float dt);
		void ResetPhysics();
		void ReloadScript();
		std::shared_ptr<Entity> AddEntity(std::string name);
		void DestroyEntity(UUID id);
		std::shared_ptr<Entity> GetEntityByID(UUID id);
		std::shared_ptr<Entity> GetEntityByName(std::string name);
		std::vector<std::shared_ptr<Entity>> GetEntities();
		void UpdateChildren();
		void UpdateWorldTransforms();
		void DrawStaticScene(const XMMATRIX& viewProjectionMatrix);
		void DrawAnimatedScene(const XMMATRIX& viewProjectionMatrix);
		void DrawWithoutCBuffer(ConstantBuffer<ModelOnly>* constantbuffer);
		void UpdateScene(float deltatime);
		void SetParent(Entity* child, Entity* parent);
		XMMATRIX GetWorldMatrix(Entity* entity);
		bool IsDescendant(Entity* possibleChild, Entity* possibleParent);
		void SetParentKeepWorld(Entity* child, Entity* newParent);
		void CreatePhysics();
		void UpdatePhysicsTransforms();
		void InitializeRuntimeResources(ID3D11Device* device, ID3D11DeviceContext* context, ConstantBuffer<CB_Anim_VS_vertexShader>& animCB, ConstantBuffer<CB_VS_vertexShader>& staticCB);
		void SerializeScene(const std::string& path);
		std::shared_ptr<Entity> CreateEntityWithUUID(UUID id, std::string name);
		void LoadScene(const std::string& path);
		bool DeserializeScene(const std::string& path);
		bool IsPlaying() const
		{
			return m_IsPlaying;
		}
		void ClearScene()
		{
			m_Entities.clear();
			m_NameToUUID.clear();
		}
		std::unique_ptr<Scene> Copy();
		Entity* GetPrimaryCameraEntity();
		XMMATRIX GetActiveCameraViewProjection(float aspectRatio);
		void UpdateRuntimeCamera(Graphics& gfx);
	private:
	
		std::unordered_map<UUID, std::shared_ptr<Entity>> m_Entities;
		std::unordered_map<std::string, UUID> m_NameToUUID;
		std::shared_ptr<Engine::PhysicsEngine> m_PhysEngine;
		ScriptEngine m_ScriptEngine;
		bool m_IsPlaying;
	};
}