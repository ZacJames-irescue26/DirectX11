#pragma once
#include "ConstantBufferTypes.h"
#include "ErrorLogger.h"
#include <nvrhi/nvrhi.h>
namespace Engine
{
template<class T>
class ConstantBuffer
{
private:
	ConstantBuffer(const ConstantBuffer<T>& rhs);

private:

	nvrhi::BufferHandle buffer;
	nvrhi::CommandListHandle deviceContext;

public:
	ConstantBuffer() {}

	T data;

	nvrhi::BufferHandle Get()const
	{
		return buffer.Get();
	}

	nvrhi::IBuffer* const* GetAddressOf()const
	{
		return buffer.GetAddressOf();
	}

	HRESULT Initialize(nvrhi::DeviceHandle device, nvrhi::CommandListHandle deviceContext)
	{
		if (buffer.Get() != nullptr)
		{
			buffer.Reset();
		}
		this->deviceContext = deviceContext;
		nvrhi::BufferDesc desc;
		desc.byteSize = static_cast<UINT>(sizeof(T));
		desc.cpuAccess = nvrhi::CpuAccessMode::Write;
		desc.isConstantBuffer = true;

		buffer = device->CreateBuffer(desc);
		return 0;
	}

	bool ApplyChanges()
	{
		deviceContext->writeBuffer(buffer, data, sizeof(T));
		return true;
	}
};
}