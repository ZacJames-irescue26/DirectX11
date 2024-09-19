#include "pch.h"
#include "ThirdPersonCamera.h"

namespace Engine
{

	void ThirdPersonCamera::LookAtPlayer(const XMFLOAT3& pos)
	{
		SetLookAtPos(pos);
	}

	void ThirdPersonCamera::SetRadius(const float& radi)
	{
		radius = radi;
	}

	data ThirdPersonCamera::Update(const XMFLOAT3& pos)
	{
		playerPos = pos;
		// Convert yaw and pitch from spherical coordinates to Cartesian coordinates
		float x = radius * cosf(Pitch) * sinf(Yaw);
		float y = radius * sinf(Pitch);
		float z = radius * cosf(Pitch) * cosf(Yaw);
		//SetRotation(x, y, z);
		//SetLookAtPos(pos);
		// Calculate the camera's position relative to the target (player)
			// Calculate the camera's position relative to the target (player)
		cameraPosition = XMFLOAT3(pos.x + x, pos.y + y, pos.z + z);

		// Update the view matrix (look at the target position from the camera position)
		XMVECTOR camPos = XMLoadFloat3(&cameraPosition);
		XMVECTOR targetPos = XMLoadFloat3(&pos);
		XMVECTOR upVector = DEFAULT_UP_VECTOR;  // Up vector (Y axis)
		
		viewMatrix = XMMatrixLookAtLH(camPos, targetPos, upVector);

		data data;
		data.campos = cameraPosition;
		data.targetpos = pos;
		data.x = x;
		data.y = y;
		data.z = z;
		data.pitch = Pitch;
		data.yaw = Yaw;
		return data;
	}


	void ThirdPersonCamera::HandleInput(float dx, float dy)
	{
		if (dx <= 0.0 || dy <= 0.0)
		{
			return;
		}
		float sensitivity = 0.01f; // Mouse sensitivity for smoother movement
		float deltax = dx - prevoiusMousePos.x;
		float deltaY = dy - prevoiusMousePos.y;
		// Adjust yaw (horizontal rotation) and pitch (vertical rotation)
		Yaw += deltax * sensitivity;
		Pitch += deltaY * sensitivity;

		// Clamp pitch to avoid flipping the camera
		const float maxPitch = XM_PI / 2.0f - 0.1f; // Slightly less than 90 degrees
		Pitch = max(-maxPitch, std::min(Pitch, maxPitch));

		prevoiusMousePos = { dx, dy };

	}

}