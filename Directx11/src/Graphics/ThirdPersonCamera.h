#pragma once
#include "Camera.h"
namespace Engine
{
	struct data 
	{
	float x;
	float y;
	float z;
	XMFLOAT3 campos;
	XMFLOAT3 targetpos;
	float pitch;
	float yaw;
	};
	class ThirdPersonCamera : public Camera
	{
	public:
		void LookAtPlayer(const XMFLOAT3& pos);
		void SetRadius(const float& radi);
		data Update(const XMFLOAT3& pos);
		void HandleInput(float dx, float dy);
		float Yaw = XMConvertToRadians(0.0f);  // Facing forward
		float Pitch = XMConvertToRadians(0.0f); // Looking straight ahead
		float radius;
		XMFLOAT3 cameraPosition;
		XMFLOAT3 playerPos;
		XMFLOAT2 prevoiusMousePos = {0,0};
	};
}