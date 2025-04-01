#include "pch.h"
#include "mesh.h"
namespace Engine
{
Mesh::Mesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<Vertex>& vertices, std::vector<DWORD>& indices, std::vector<Texture> tex, const XMMATRIX& Matrix)
{
	this->deviceContext = deviceContext;
	this->textures = tex;
	this->transformMatrix = Matrix;
	HRESULT hr = this->vertexbuffer.Initialize(device, vertices.data(), vertices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize vertex buffer for mesh.");

	hr = this->indexbuffer.Initialize(device, indices.data(), indices.size());
	COM_ERROR_IF_FAILED(hr, "Failed to initialize index buffer for mesh.");
}

Mesh::Mesh(const Mesh& mesh)
{
	this->deviceContext = mesh.deviceContext;
	this->indexbuffer = mesh.indexbuffer;
	this->vertexbuffer = mesh.vertexbuffer;
	this->textures = mesh.textures;
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
			break;
		case aiTextureType_NORMALS:
			normalSRV = tex.GetTextureResourceView();
			break;
		case aiTextureType_DIFFUSE_ROUGHNESS:
			roughnessSRV = tex.GetTextureResourceView();
			break;
		default:
			assert(false);// Optional: bind a default color texture or ignore
			break;
		}
	}

	// Always bind even if they're nullptr
	deviceContext->PSSetShaderResources(0, 1, &diffuseSRV);   // t0
	deviceContext->PSSetShaderResources(1, 1, &normalSRV);    // t1
	deviceContext->PSSetShaderResources(2, 1, &roughnessSRV); // t2

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
