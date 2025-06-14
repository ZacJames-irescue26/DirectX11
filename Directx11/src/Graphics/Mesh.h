#pragma once

#include "Vertex.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"

#include "Texture.h"
namespace Engine
{
class Mesh
{
public:
	Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, std::vector<Texture> tex, const XMMATRIX& transformMatrix);
	void DrawJustMesh();
	Mesh(const Mesh& mesh);
	void Draw();
	const XMMATRIX& GetTransformMatrix();
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	std::vector<Texture> textures;
private:
	VertexBuffer<Vertex> vertexbuffer;
	IndexBuffer indexbuffer;
	ID3D11DeviceContext* deviceContext;
	XMMATRIX transformMatrix;
};
}