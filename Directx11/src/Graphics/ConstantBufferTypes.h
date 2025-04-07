#pragma once
#include <DirectXMath.h>
namespace Engine
{
struct alignas(16) CB_VS_vertexShader
{
	// need to align to 16 bytes
	
	XMMATRIX wvpMatrix;
	XMMATRIX worldMatrix;
	XMMATRIX worldInvTransposeMatrix;
};
struct CB_VS_ViewProj
{
	XMMATRIX Projection;
	XMMATRIX View;
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
	XMMATRIX InvProj;
	XMMATRIX InvView;
	XMFLOAT3 CameraPosition;
	float padding=0.0;
};

struct DirectionalLightParams
{
	DirectX::XMFLOAT3 LightColor;
	float padding = 0.0;
	DirectX::XMFLOAT3 LightDirection;
	float padding1 = 0.0;
};

struct PrefilteringParams
{
	float roughness;
	DirectX::XMFLOAT3 padding;
};

}
