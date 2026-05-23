#pragma once
#include <DirectXMath.h>
#include "BoneData.h"
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
struct AnimatedVertex
{
	AnimatedVertex() {}
	AnimatedVertex(XMFLOAT3 position, XMFLOAT2 uv, XMFLOAT3 norm)
		: pos(position), texCoord(uv), normal(norm) {
	}

	XMFLOAT3 pos;
	XMFLOAT2 texCoord;
	XMFLOAT3 normal;
	XMFLOAT3 Tangent;
	XMFLOAT3 BiTangent;
	//bone indexes which will influence this vertex
	int m_BoneIDs[MAXBONEPERVERTEX];
	//weights from each bone
	float m_Weights[MAXBONEPERVERTEX];

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
	SurfelVB(XMFLOAT4 position, XMFLOAT4 normal, XMFLOAT4 color, float radius)
		: pos(position), norm(normal), color(color), radius(radius) {}


	XMFLOAT4 pos;
	XMFLOAT4 norm;
	XMFLOAT4 color;
	XMFLOAT3 indirectRadiance;
	float radius;
};
}