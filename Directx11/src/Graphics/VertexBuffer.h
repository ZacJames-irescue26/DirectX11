#pragma once
#include <nvrhi/d3d12.h>
namespace Engine
{
template<class T>
class VertexBuffer
{
private:

private:
	nvrhi::BufferHandle buffer;
	UINT stride = sizeof(T);
	UINT bufferSize = 0;

public:
	VertexBuffer() {}
	VertexBuffer<T>& operator=(const VertexBuffer<T>& a)
	{
		this->buffer = a.buffer;
		this->bufferSize = a.bufferSize;
		this->stride = a.stride;
		return *this;
	}
	VertexBuffer(const VertexBuffer<T>& rhs)
	{
		this->buffer = rhs.buffer;
		this->bufferSize = rhs.bufferSize;
		this->stride = rhs.stride;
	}

	nvrhi::BufferHandle Get()const
	{
		return buffer.Get();
	}

	nvrhi::IBuffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	UINT BufferSize() const
	{
		return this->bufferSize;
	}

	const UINT Stride() const
	{
		return this->stride;
	}

	const UINT* StridePtr() const
	{
		return &stride;
	}

	HRESULT Initialize(nvrhi::DeviceHandle* device, nvrhi::CommandListHandle commandlist, T* data, UINT numVertices)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}
		this->bufferSize = numVertices;

		nvrhi::BufferDesc vertexBufferDesc;
		vertexBufferDesc.byteSize = sizeof(T)* numVertices;
		vertexBufferDesc.isVertexBuffer = true;

		buffer = device->CreateBuffer(vertexBufferDesc);
		
		commandlist->writeBuffer(buffer, data, numVertices)
		
		return 0;
	}
};
}