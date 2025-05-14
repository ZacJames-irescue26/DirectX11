#include "pch.h"
#include "PhysicsObject.h"

namespace Engine
{
	bool PhysicsObject::Initialize(BodyID id, const std::string& filePath, nvrhi::DeviceHandle* device, nvrhi::CommandListHandle* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader)
	
	{
		Id = id;
		return GameObject::Initialize(filePath, device, deviceContext, cb_vs_vertexshader);
	}

}