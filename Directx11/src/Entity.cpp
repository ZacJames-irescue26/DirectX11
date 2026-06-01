#include "pch.h"
#include "Entity.h"
#include "src/Scene/Scene.h"


namespace Engine
{

	Entity* Entity::GetChildByName(const std::string& name)
	{

		if (!m_Scene)
			return nullptr;

		for (UUID childID : GetChildren())
		{
			auto child = GetScene()->GetEntityByID(childID);

			if (child && child->GetName() == name)
				return child.get();
		}

		return nullptr;
	}

}