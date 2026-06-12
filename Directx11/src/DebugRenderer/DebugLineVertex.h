#pragma once

#include <DirectXMath.h>
namespace Engine
{
	struct DebugLineVertex
	{
		XMFLOAT3 Position;
		XMFLOAT3 Color;
		float RemainingTime = 0.0f;

	};
}