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
			entity->AddComponent<TransformComponent>(std::make_unique<TransformComponent>());
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
				auto* mesh = entity->GetComponent<StaticMeshComponent>();

				if (!mesh)
					continue;

				XMMATRIX world = GetWorldMatrix(entity.get());

				mesh->Draw(world, viewProjectionMatrix);
			}
		}
		void DrawAnimatedScene(const XMMATRIX& viewProjectionMatrix)
		{
			for (auto& [id, entity] : m_Entities)
			{
				auto* animMesh = entity->GetComponent<AnimatedMeshComponent>();

				if (!animMesh)
					continue;

				XMMATRIX world = GetWorldMatrix(entity.get());

				animMesh->Draw(world, viewProjectionMatrix);
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
		void UpdateScene(float deltatime)
		{
			for (auto& [id, entity] : m_Entities)
			{
				if (entity->GetComponent<AnimatedMeshComponent>())
				{
					entity->GetComponent<AnimatedMeshComponent>()->Update(deltatime);
				}
			}
		
		
		}

		void SetParent(Entity* child, Entity* parent)
		{

			if (parent && IsDescendant(parent, child))
			{
				// Would create cycle.
				return;
			}

			if (!child)
				return;

			UUID oldParentId = child->GetParent();

			if (oldParentId != 0)
			{
				auto oldParent = GetEntityByID(oldParentId);
				if (oldParent)
					oldParent->RemoveChild(child->GetUUID());
			}

			if (parent)
			{
				child->SetParent(parent->GetUUID());
				parent->AddChild(child->GetUUID());
			}
			else
			{
				child->SetParent(0);
			}
		}
		XMMATRIX GetWorldMatrix(Entity* entity)
		{
			if (!entity)
				return XMMatrixIdentity();

			auto* transform = entity->GetComponent<TransformComponent>();

			XMMATRIX local =
				transform
				? transform->ModelMatrix
				: XMMatrixIdentity();

			UUID parentId = entity->GetParent();

			if (parentId == 0)
				return local;

			auto parent = GetEntityByID(parentId);

			if (!parent)
				return local;

			XMMATRIX parentWorld = GetWorldMatrix(parent.get());

			// Row-vector convention:
			return local * parentWorld;
		}

		bool IsDescendant(Entity* possibleChild, Entity* possibleParent)
		{
			if (!possibleChild || !possibleParent)
				return false;

			UUID parentId = possibleChild->GetParent();

			while (parentId != 0)
			{
				if (parentId == possibleParent->GetUUID())
					return true;

				auto parent = GetEntityByID(parentId);

				if (!parent)
					return false;

				parentId = parent->GetParent();
			}

			return false;
		}

		void SetParentKeepWorld(Entity* child, Entity* newParent)
		{
			if (!child)
				return;

			auto* transform = child->GetComponent<TransformComponent>();
			if (!transform)
				return;

			XMMATRIX oldWorld = GetWorldMatrix(child);

			SetParent(child, newParent);

			XMMATRIX parentWorld = XMMatrixIdentity();

			if (newParent)
				parentWorld = GetWorldMatrix(newParent);

			XMMATRIX newLocal = oldWorld * XMMatrixInverse(nullptr, parentWorld);

			// You need to decompose back into Position/Rotation/Scale.
			XMVECTOR scale;
			XMVECTOR rotation;
			XMVECTOR translation;

			XMMatrixDecompose(&scale, &rotation, &translation, newLocal);

			XMStoreFloat3(&transform->Scale, scale);
			XMStoreFloat3(&transform->Position, translation);

			XMFLOAT4 q;
			XMStoreFloat4(&q, rotation);

			// Convert quaternion to Euler if your TransformComponent stores Euler.
			// Easier long-term: store rotation as quaternion.
		}
	private:
	
		std::unordered_map<UUID, std::shared_ptr<Entity>> m_Entities;
		std::unordered_map<std::string, UUID> m_NameToUUID;

	};
}