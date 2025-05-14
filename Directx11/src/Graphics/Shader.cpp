#include "pch.h"
#include "Shader.h"
#include <nvrhi/d3d12.h>
#include <nvrhi/nvrhi.h>

namespace Engine
{
bool VertexShader::Initialize(nvrhi::DeviceHandle device, LPCWSTR shaderpath, std::vector<nvrhi::VertexAttributeDesc> desc, UINT elements)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath, this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::string path = "";
		char pBuf[1024];

		_getcwd(pBuf, 1024);
		path = pBuf;
		path += " working directory";

		std::wstring wpath = std::wstring(path.begin(), path.end());
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath; 
		errorMsg.append(wpath);
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}
	nvrhi::ShaderDesc shaderdesc;
	shaderdesc.shaderType = nvrhi::ShaderType::Vertex;
	shaderdesc.entryName = "main";

	shader = device->createShader(shaderdesc, shader_buffer.Get()->GetBufferPointer(), shader_buffer.Get()->GetBufferSize());

	inputLayout = device->createInputLayout(desc.data(), elements, shader);
	return true;
}

nvrhi::ShaderHandle VertexShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* VertexShader::GetBuffer()
{
	return this->shader_buffer.Get();
}

nvrhi::InputLayoutHandle VertexShader::GetInputLayout()
{
	return inputLayout.Get();
}

bool PixelShader::Initialize(nvrhi::DeviceHandle device, std::wstring shaderpath)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}
	nvrhi::ShaderDesc desc;
	desc.entryName = "main";
	desc.shaderType = nvrhi::ShaderType::Pixel;
	
	shader = device->createShader(desc, this->shader_buffer.Get()->GetBufferPointer(), this->shader_buffer.Get()->GetBufferSize());

	return true;
}

nvrhi::ShaderHandle PixelShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* PixelShader::GetBuffer()
{
	return this->shader_buffer.Get();
}
bool GeometryShader::Initialize(nvrhi::DeviceHandle device, std::wstring shaderpath)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	nvrhi::ShaderDesc desc;
	desc.entryName = "main";
	desc.shaderType = nvrhi::ShaderType::Geometry;

	shader = device->createShader(desc, this->shader_buffer.Get()->GetBufferPointer(), this->shader_buffer.Get()->GetBufferSize());

	return true;
}

nvrhi::ShaderHandle GeometryShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* GeometryShader::GetBuffer()
{
	return this->shader_buffer.Get();
}
bool ComputeShader::Initialize(nvrhi::DeviceHandle device, std::wstring shaderpath)
{
	HRESULT hr = D3DReadFileToBlob(shaderpath.c_str(), this->shader_buffer.GetAddressOf());
	if (FAILED(hr))
	{
		std::wstring errorMsg = L"Failed to load shader: ";
		errorMsg += shaderpath;
		ErrorLogger::Log(hr, errorMsg);
		return false;
	}

	nvrhi::ShaderDesc desc;
	desc.entryName = "main";
	desc.shaderType = nvrhi::ShaderType::Compute;

	shader = device->createShader(desc, this->shader_buffer.Get()->GetBufferPointer(), this->shader_buffer.Get()->GetBufferSize());

	return true;
}

nvrhi::ShaderHandle ComputeShader::GetShader()
{
	return this->shader.Get();
}

ID3D10Blob* ComputeShader::GetBuffer()
{
	return this->shader_buffer.Get();
}
}


