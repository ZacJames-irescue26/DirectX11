#pragma once
#include <DirectXMath.h>
namespace Engine
{

	struct CB_Anim_VS_vertexShader
	{
		XMMATRIX wvpMatrix;
		XMMATRIX worldMatrix;
		XMMATRIX worldInvTransposeMatrix;
		XMMATRIX Bones[100];
	};


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
	XMFLOAT3 ambientLightColor;
	float ambientLightStrength;

	XMFLOAT3 dynamicLightColor;
	float dynamicLightStrength;
	XMFLOAT3 dynamicLightPosition;
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

	XMFLOAT3 LightColor;
	float padding = 0.0;
	XMFLOAT3 LightDirection;
	float farplane =1000;
	XMFLOAT4 cascadePlaneDistances;
	XMMATRIX LightSpaceMatrices0;
	XMMATRIX LightSpaceMatrices1;
	XMMATRIX LightSpaceMatrices2;
	XMMATRIX LightSpaceMatrices3;
	XMMATRIX LSMDirectionalShadow;
	
};

struct PrefilteringParams
{
	float roughness;
	XMFLOAT3 padding;
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

struct SurfelCSBUffer
{
	uint screenWidth;
	uint screenHeight;
	float padding;
	float padding1;
	XMMATRIX viewProj;
	XMFLOAT2 screenSize;
	uint tileCountX;
	uint tileCountY;
};
struct SurfelCreation
{
	XMMATRIX InvViewProj[6];

	XMFLOAT3 ProbePosition;
	float CaptureRadius;

	uint32_t FaceSize;
	uint32_t SourceProbeIndex;
	uint32_t ReversedZ;
	uint32_t Padding0;
};


struct ComputeLightCB
{
	XMFLOAT3 LightDirection;
	float  LightIntensity;

	XMFLOAT3 LightColor;
	uint32_t SurfelCount;

	uint32_t ProbeCount;
	float  RadianceScale;
	float  ProbeInfluenceRadius;
	float  Padding0;
	XMMATRIX LightSpaceMatrix;
};

struct ComputeNormalizeCB
{
	uint32_t ProbeCount;
	float InvRadianceScale;
	XMFLOAT2 Padding;
};

struct ProbeVolumeCB
{
	uint32_t ProbeCount;
	uint32_t UseGI;
	uint32_t ShowGIOnly;
	float Padding;
};

}
