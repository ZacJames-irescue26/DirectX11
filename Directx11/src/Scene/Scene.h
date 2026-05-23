#pragma once 
#include "Entity.h"
#include "UUID.h"

namespace Engine
{

	class Scene
	{
	public:

		std::shared_ptr<Entity> AddEntity(std::string name)
		{
			UUID id{};
			std::shared_ptr<Entity> entity = std::make_shared<Entity>(name, id);
			entity->AddComponent(std::make_unique<TransformComponent>());
			m_Entities.insert({id, entity});
			m_NameToUUID.insert({name, id});
			return entity;
		}
		void DestroyEntity(UUID id)
		{
			m_NameToUUID.erase(m_Entities.find(id)->second->GetName());
			m_Entities.erase(id);

		}

		std::shared_ptr<Entity> GetEntityByID(UUID id)
		{
			return m_Entities.find(id)->second;
		}
		std::shared_ptr<Entity> GetEntityByName(std::string name)
		{
			return m_Entities.find(m_NameToUUID.find(name)->second)->second;
		}
		std::vector<std::shared_ptr<Entity>> GetEntities()
		{
			std::vector<std::shared_ptr<Entity>> outvec;
			for (const auto& [id, entity] : m_Entities)
			{
				outvec.push_back(entity);
			}
			return outvec;
		}
		void DrawStaticScene(const XMMATRIX& viewProjectionMatrix)
		{
			for (auto& [id, entity] : m_Entities)
			{
				if (!entity->HasComponent<TransformComponent>())
				{
					return;
				}
				if (entity->GetComponent<StaticMeshComponent>())
				{
					entity->GetComponent<StaticMeshComponent>()->Draw(entity->GetComponent<TransformComponent>()->CalculateModelMatrix(), viewProjectionMatrix);
				}
			}
		}

		void DrawWithoutCBuffer(ConstantBuffer<ModelOnly>* constantbuffer)
		{
			for (auto& [id, entity] : m_Entities)
			{

				constantbuffer->data.Model = entity->GetComponent<TransformComponent>()->CalculateModelMatrix();
				constantbuffer->ApplyChanges();
				if (entity->GetComponent<StaticMeshComponent>())
				{
					entity->GetComponent<StaticMeshComponent>()->DrawWithoutCBuffer();
				}
			}
		}
	private:
	
		std::unordered_map<UUID, std::shared_ptr<Entity>> m_Entities;
		std::unordered_map<std::string, UUID> m_NameToUUID;

	};
}