#pragma once
#include "ErrorLogger.h"
#include <nvrhi/nvrhi.h>
namespace Engine
{
class VertexShader
{
public:
	bool Initialize(nvrhi::DeviceHandle device, LPCWSTR shaderpath, std::vector<nvrhi::VertexAttributeDesc> desc, UINT elements);
	nvrhi::ShaderHandle GetShader();
	ID3D10Blob* GetBuffer();
	nvrhi::InputLayoutHandle GetInputLayout();
private:
	nvrhi::ShaderHandle shader;
	Microsoft::WRL::ComPtr<ID3D10Blob> shader_buffer;
	nvrhi::InputLayoutHandle inputLayout;


};

class PixelShader
{
public:
	bool Initialize(nvrhi::DeviceHandle device, std::wstring shaderpath);
	nvrhi::ShaderHandle GetShader();
	ID3D10Blob* GetBuffer();
private:
	nvrhi::ShaderHandle shader;
	Microsoft::WRL::ComPtr<ID3D10Blob> shader_buffer;
};

class GeometryShader
{
public:
	bool Initialize(nvrhi::DeviceHandle device, std::wstring shaderpath);
	nvrhi::ShaderHandle GetShader();
	ID3D10Blob* GetBuffer();
private:
	nvrhi::ShaderHandle shader;
	Microsoft::WRL::ComPtr<ID3D10Blob> shader_buffer;
};
class ComputeShader
{
public:
	bool Initialize(nvrhi::DeviceHandle device, std::wstring shaderpath);
	nvrhi::ShaderHandle GetShader();
	ID3D10Blob* GetBuffer();
private:
	nvrhi::ShaderHandle shader;
	Microsoft::WRL::ComPtr<ID3D10Blob> shader_buffer;
};

}
