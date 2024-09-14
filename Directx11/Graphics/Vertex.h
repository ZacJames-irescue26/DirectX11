#pragma once
#include <DirectXMath.h>
namespace Engine
{
struct Vertex
{
	Vertex() {}
	Vertex(XMFLOAT3 position, XMFLOAT2 uv, XMFLOAT3 norm)
		: pos(position), texCoord(uv), normal(norm) {}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
};
}