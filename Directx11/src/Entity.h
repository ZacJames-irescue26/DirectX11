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
	class Entity
	{

	public:
		Entity(std::string name, UUID id)
			: m_id(id), name(std::move(name))
		{

		}
		template<typename T>
		void AddComponent(std::unique_ptr<Component> comp)
		{
			if (!comp && !HasComponent<T>())
				return;

			m_ComponentTypes.push_back(comp->GetType());
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
		UUID GetUUID() const
		{
			return m_id;
		}
		std::string& GetName() 
		{
			return name;
		}
	private:

		UUID m_id;
		std::string name;
		std::vector<std::unique_ptr<Component>> m_Components;
		std::vector<ComponentEnum> m_ComponentTypes;
	};
}