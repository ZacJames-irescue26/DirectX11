#include "pch.h"
#include "PhysicsObject.h"

namespace Engine
{
	bool PhysicsObject::Initialize(BodyID id, const std::string& filePath, ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader)
	
	{
		Id = id;
		return GameObject::Initialize(filePath, device, deviceContext, cb_vs_vertexshader);
	}

}