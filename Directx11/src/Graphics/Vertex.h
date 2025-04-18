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
	XMFLOAT3 Tangent;
	XMFLOAT3 BiTangent;
};

struct FullScreenQuad
{
	FullScreenQuad() {}
	FullScreenQuad(XMFLOAT2 position, XMFLOAT2 uv)
	: pos(position), texcoord(uv){}


	XMFLOAT2 pos;
	XMFLOAT2 texcoord;

};
struct CubeWPos
{
	CubeWPos() {}
	CubeWPos(XMFLOAT3 pos) 
	:pos(pos){}

	XMFLOAT3 pos;

};

struct SurfelVB
{
	SurfelVB() {}
	SurfelVB(XMFLOAT3 position, XMFLOAT3 normal, XMFLOAT4 color, float radius)
		: pos(position), norm(normal), color(color), radius(radius) {}


	XMFLOAT3 pos;
	XMFLOAT3 norm;
	XMFLOAT4 color;
	float radius;
};
}