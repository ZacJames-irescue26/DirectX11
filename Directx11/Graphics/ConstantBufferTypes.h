#pragma once
#include <DirectXMath.h>
namespace Engine
{
struct CB_VS_vertexShader
{
	// need to align to 16 bytes
	
	XMMATRIX wvpMatrix;
	XMMATRIX worldMatrix;
};

struct CB_FS_LightPos
{
	DirectX::XMFLOAT3 ambientLightColor;
	float ambientLightStrength;

	DirectX::XMFLOAT3 dynamicLightColor;
	float dynamicLightStrength;
	DirectX::XMFLOAT3 dynamicLightPosition;
};
struct CameraInfo
{
	XMFLOAT3 CameraPosition;
};

struct LightParams
{
	DirectX::XMFLOAT3 LightPos;
	DirectX::XMFLOAT3 LightColor;
	DirectX::XMFLOAT3 LightDirection;
	DirectX::XMFLOAT2 SpotlightAngles;
	DirectX::XMFLOAT4 LightRange;
};


}
