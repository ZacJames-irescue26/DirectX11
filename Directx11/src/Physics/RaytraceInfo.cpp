#include "pch.h"
#include "RaytraceInfo.h"

namespace Engine
{
	void RayHit::Clear()
	{
		HitEntity = 0;
		Position = XMFLOAT3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		Normal = XMFLOAT3(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		Distance = std::numeric_limits<float>::max();
		HitCollider = nullptr;
	}
}