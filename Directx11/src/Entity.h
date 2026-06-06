#pragma once
#include "Components\Components.h"
#include "UUID.h"
#include <type_traits>
#include <vector>
#include <memory>
#include <string>
#include <utility>
namespace Engine
{
	class Scene;

	class Entity
	{

	public:
		Entity(std::string name, UUID id, Scene* scene)
			: m_id(id), name(std::move(name)), m_Scene(scene)
		{

		}

		std::shared_ptr<Entity> Clone(Entity* old)
		{
			auto copy = std::make_shared<Entity>(name, m_id, m_Scene);

			copy->m_Parent = m_Parent;
			copy->m_Children = m_Children;
			int i= 0;
			for (const auto& component : m_Components)
			{
				
				if (component)
					copy->AddComponent(component->Clone());
			
			
				i++;
			}

			return copy;
		}

		void AddComponent(std::unique_ptr<Component> comp)
		{
			if (!comp)
				return;

			ComponentEnum type = comp->GetType();

			for (ComponentEnum existing : m_ComponentTypes)
			{
				if (existing == type)
					return;
			}

			m_ComponentTypes.push_back(type);
			m_Components.push_back(std::move(comp));
		}

		

		template<typename T>
		void RemoveComponent()
		{
			static_assert(std::is_base_of<Component, T>::value,
				"T must inherit from Component");

			for (size_t i = 0; i < m_ComponentTypes.size(); ++i)
			{
				if (m_ComponentTypes[i] == T::StaticType())
				{
					m_ComponentTypes.erase(m_ComponentTypes.begin() + i);
					m_Components.erase(m_Components.begin() + i);
					return;
				}
			}
		}
		Engine::UUID GetParent() const { return m_Parent; }

		void SetParent(Engine::UUID parent)
		{
			m_Parent = parent;
		}

		const std::vector<Engine::UUID>& GetChildren() const
		{
			return m_Children;
		}

		void AddChild(Engine::UUID child)
		{
			if (std::find(m_Children.begin(), m_Children.end(), child) == m_Children.end())
				m_Children.push_back(child);
		}

		void RemoveChild(Engine::UUID child)
		{
			m_Children.erase(
				std::remove(m_Children.begin(), m_Children.end(), child),
				m_Children.end()
			);
		}

		Scene* GetScene()
		{
			return m_Scene;
		}

		const Scene* GetScene() const
		{
			return m_Scene;
		}
		Entity* GetChildByName(const std::string& name);

		template<typename T>
		T* GetComponent()
		{
			static_assert(std::is_base_of<Component, T>::value,
				"T must inherit from Component");

			for (size_t i = 0; i < m_Components.size(); ++i)
			{
				T* casted = dynamic_cast<T*>(m_Components[i].get());

				if (casted)
					return casted;
			}

			return nullptr;
		}
		template<typename T>
		bool HasComponent()
		{
			static_assert(std::is_base_of<Component, T>::value,
				"T must inherit from Component");

			for (size_t i = 0; i < m_ComponentTypes.size(); ++i)
			{
				if (m_ComponentTypes[i] == T::StaticType())
				{
					return true;
				}
			}

			return false;
		}
		Engine::UUID GetUUID() const
		{
			return m_id;
		}
		std::string GetUUIDString() const
		{
			return std::to_string(m_id);
		}
		std::string& GetName() 
		{
			return name;
		}
	private:
		Scene* m_Scene = nullptr;

		Engine::UUID m_id;
		std::string name;
		std::vector<std::unique_ptr<Component>> m_Components;
		std::vector<ComponentEnum> m_ComponentTypes;
		Engine::UUID m_Parent = 0;
		std::vector<Engine::UUID> m_Children;

	};
}