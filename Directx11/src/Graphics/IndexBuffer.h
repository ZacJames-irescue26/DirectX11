#pragma once
#include <nvrhi\nvrhi.h>
namespace Engine
{
class IndexBuffer
{
private:

private:
	nvrhi::BufferHandle buffer;
	UINT bufferSize = 0;
public:
	IndexBuffer(const IndexBuffer& rhs) {}
	IndexBuffer() {}

	nvrhi::IBuffer* Get()const
	{
		return buffer.Get();
	}
	nvrhi::BufferHandle GetComPtr()
	{
		return buffer;
	}
	nvrhi::IBuffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	UINT BufferSize() const
	{
		return this->bufferSize;
	}

	HRESULT Initialize(nvrhi::DeviceHandle device, nvrhi::CommandListHandle commandlist, DWORD* data, UINT numIndices)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}
		this->bufferSize = numIndices;
		//Load Index Data
		nvrhi::BufferDesc indexBufferDesc = {};
		indexBufferDesc.isIndexBuffer = true;
		indexBufferDesc.byteSize = sizeof(DWORD) * numIndices;
		buffer = device->createBuffer(indexBufferDesc);
		commandlist->writeBuffer(buffer, data, bufferSize);
		return 0;
	}
};
}