#include "pch.h"
#include "mesh.h"
#include "Graphics.h"
namespace Engine
{
Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& Vertices, std::vector<DWORD>& indices, std::vector<Texture> tex, const XMMATRIX& Matrix)
{
	this->deviceContext = deviceContext;
	this->textures = tex;
	this->transformMatrix = Matrix;
	for (const auto& vert : Vertices)
	{
		this->vertices.push_back(vert);
	}
	for (const auto& indi: indices)
	{
		this->indices.push_back(indi);
	}
	HRESULT hr = this->vertexbuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexbuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}
//std::vector<Vertex> ReadVertexBuffer(
//	ID3D11Device* device,
//	ID3D11DeviceContext* context,
//	ID3D11Buffer* vertexBuffer)
//{
//	std::vector<Vertex> result;
//
//	// Step 1: Get buffer description
//	D3D11_BUFFER_DESC desc = {};
//	vertexBuffer->GetDesc(&desc);
//
//	// Step 2: Create staging buffer with CPU read access
//	D3D11_BUFFER_DESC stagingDesc = desc;
//	stagingDesc.Usage = D3D11_USAGE_STAGING;
//	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//	stagingDesc.BindFlags = 0;
//	stagingDesc.MiscFlags = 0;
//
//	Microsoft::WRL::ComPtr<ID3D11Buffer> stagingBuffer;
//	HRESULT hr = device->CreateBuffer(&stagingDesc, nullptr, &stagingBuffer);
//	if (FAILED(hr))
//	{
//		// Handle error
//		return result;
//	}
//
//	// Step 3: Copy the vertex buffer to the staging buffer
//	context->CopyResource(stagingBuffer.Get(), vertexBuffer);
//
//	// Step 4: Map the buffer to read its contents
//	D3D11_MAPPED_SUBRESOURCE mappedResource;
//	hr = context->Map(stagingBuffer.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
//	if (FAILED(hr))
//	{
//		// Handle error
//		return result;
//	}
//
//	// Step 5: Copy data to vector
//	size_t vertexCount = desc.ByteWidth / sizeof(Vertex);
//	result.resize(vertexCount);
//	memcpy(result.data(), mappedResource.pData, desc.ByteWidth);
//
//	// Unmap after reading
//	context->Unmap(stagingBuffer.Get(), 0);
//
//	return result;
//}

Mesh::Mesh(const Mesh& mesh)
{
	
	this->vertices = mesh.vertices;
	this->deviceContext = mesh.deviceContext;
	this->indexbuffer = mesh.indexbuffer;
	this->vertexbuffer = mesh.vertexbuffer;
	this->textures = mesh.textures;
	this->indices = mesh.indices;

}
void Mesh::DrawJustMesh()
{
	UINT offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, vertexbuffer.GetAddressOf(), vertexbuffer.StridePtr(), &offset);
	deviceContext->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(indexbuffer.BufferSize(), 0, 0);
}
void Mesh::Draw()
{
	UINT offset = 0;

	ID3D11ShaderResourceView* diffuseSRV = nullptr;
	ID3D11ShaderResourceView* normalSRV = nullptr;
	ID3D11ShaderResourceView* roughnessSRV = nullptr;

	for (auto& tex : textures)
	{
		switch (tex.GetType())
		{
		case aiTextureType_DIFFUSE:
			diffuseSRV = tex.GetTextureResourceView();
			deviceContext->PSSetShaderResources(0, 1, &diffuseSRV);   // t0
			break;
		case aiTextureType_NORMALS:
			normalSRV = tex.GetTextureResourceView();
			deviceContext->PSSetShaderResources(1, 1, &normalSRV);    // t1
			break;
		case aiTextureType_DIFFUSE_ROUGHNESS:
			roughnessSRV = tex.GetTextureResourceView();
			deviceContext->PSSetShaderResources(2, 1, &roughnessSRV); // t2
			break;
		default:
			assert(false);// Optional: bind a default color texture or ignore
			break;
		}
	}

	// Always bind even if they're nullptr

	// Bind vertex and index buffers
	deviceContext->IASetVertexBuffers(0, 1, vertexbuffer.GetAddressOf(), vertexbuffer.StridePtr(), &offset);
	deviceContext->IASetIndexBuffer(indexbuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	deviceContext->DrawIndexed(indexbuffer.BufferSize(), 0, 0);

	ID3D11ShaderResourceView* nullSRVs[3] = { nullptr, nullptr, nullptr };
	this->deviceContext->PSSetShaderResources(0, 3, nullSRVs);

}
const XMMATRIX& Mesh::GetTransformMatrix()
{
	return this->transformMatrix;
}






}
