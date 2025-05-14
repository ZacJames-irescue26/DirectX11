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
	Mesh(nvrhi::DeviceHandle device, nvrhi::CommandListHandle deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, std::vector<Texture> tex, const XMMATRIX& transformMatrix);
	void DrawJustMesh(nvrhi::CommandListHandle commandList);
	Mesh(const Mesh& mesh);
	void Draw(nvrhi::DeviceHandle device, nvrhi::CommandListHandle commandList, nvrhi::BindingSetDesc& bindingSetDesc);
	const XMMATRIX& GetTransformMatrix();
	std::vector<Vertex> vertices;
	std::vector<DWORD> indices;
	std::vector<Texture> textures;
private:
	VertexBuffer<Vertex> vertexbuffer;
	IndexBuffer indexbuffer;
	nvrhi::CommandListHandle deviceContext;
	XMMATRIX transformMatrix;
};
}