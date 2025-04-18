#pragma once
namespace Engine
{
	// Define the Ray structure
	struct Ray {
		XMVECTOR origin;
		XMVECTOR direction;
	};

	// Define the Intersection structure
	struct Intersection {
		bool hit = false;
		float distance = std::numeric_limits<float>::max();
		XMFLOAT3 point;
		XMFLOAT3 normal;
		XMFLOAT3 albedo; // Material of the hit surface
	};

}