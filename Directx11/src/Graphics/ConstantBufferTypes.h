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
	XMMATRIX View;
	XMFLOAT3 CameraPosition;
	float padding=0.0;
};

struct DirectionalLightParams
{

	DirectX::XMFLOAT3 LightColor;
	float padding = 0.0;
	DirectX::XMFLOAT3 LightDirection;
	float farplane =1000;
	XMFLOAT4 cascadePlaneDistances;
	XMMATRIX LightSpaceMatrices0;
	XMMATRIX LightSpaceMatrices1;
	XMMATRIX LightSpaceMatrices2;
	XMMATRIX LightSpaceMatrices3;
	
};

struct PrefilteringParams
{
	float roughness;
	DirectX::XMFLOAT3 padding;
};

struct LightSpaceMatrices
{
	XMMATRIX LightSpace;
};
struct ModelOnly
{
	XMMATRIX Model;
};

struct DebugColors
{
	XMFLOAT4 Red = { 1.0,0.0,0.0,1.0 };
	XMFLOAT4 Green = { 0.0,1.0,0.0,1.0 };
	XMFLOAT4 Blue = { 0.0,0.0,1.0,1.0 };
	uint32_t index = 0;
};
struct CastLight
{
	XMFLOAT3 intensity;
	float padding;
	XMFLOAT3 position;
	float padding1;
	XMFLOAT3 direction;
	float cutOff;
};
struct Lights
{
	CastLight light;
};

struct ShadowlightingInfo
{
	XMMATRIX LightViewProj; // same as your raster shadow’s VP
	XMMATRIX InvLightViewProj; // inverse of the above
	float Bias; // depth bias
	uint MapSize; // resolution
	uint NumNodes;
	uint NumTris;
	XMFLOAT3 LightDir;
	float pad;
};
struct alignas(16) BaseCB{ 

	uint32_t NumBases;
	XMFLOAT3 pad;

};



}
