#pragma once
#include "Game/GameObject.h"

namespace Engine
{

	class Light : public GameObject
	{
	public:
		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, ConstantBuffer<CB_VS_vertexShader>& cb_vs_vertexshader);

		XMFLOAT3 lightColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		float lightStrength = 1.0f;
	};

}