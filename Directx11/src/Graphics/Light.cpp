#include "pch.h"
#include "Light.h"

namespace Engine
{
	bool Light::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader)
	{
		if (!model.Initialize("Assets/TestCube.gltf", device, deviceContext, cb_vs_vertexshader))
			return false;

		this->SetPosition(0.0f, 0.0f, 0.0f);
		this->SetRotation(0.0f, 0.0f, 0.0f);
		this->UpdateMatrix();
		return true;
	}



}