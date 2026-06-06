#pragma once
#include <DirectXMath.h>
#include <complex>

struct ScriptMath
{
	static XMFLOAT3 ForwardFromYawPitch(float yaw, float pitch)
	{
		float cp = std::cos(pitch);
		return XMFLOAT3(
			std::sin(yaw) * cp,
			-std::sin(pitch),
			std::cos(yaw) * cp
		);
	}

	static XMFLOAT3 RightFromYaw(float yaw)
	{
		return XMFLOAT3(
			std::cos(yaw),
			0.0f,
			-std::sin(yaw)
		);
	}

	static XMFLOAT3 Normalize(const XMFLOAT3& v)
	{
		XMVECTOR vec = XMLoadFloat3(&v);
		vec = XMVector3Normalize(vec);

		XMFLOAT3 out;
		XMStoreFloat3(&out, vec);
		return out;
	}
};