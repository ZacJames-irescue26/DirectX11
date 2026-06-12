#include "pch.h"
#include "DebugRenderer.h"
#include "Scene\Project.h"
#include "InputElements.h"

namespace Engine
{

	bool DebugRenderer::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext, uint32_t maxLines /*= 65536*/)
	{
		if (!device)
			return false;

		m_MaxLines = maxLines;
		m_LineVertices.reserve(static_cast<size_t>(maxLines) * 2);
		m_PermaLineVertices.reserve(static_cast<size_t>(maxLines));
		D3D11_BUFFER_DESC vbDesc = {};
		vbDesc.ByteWidth = sizeof(DebugLineVertex) * maxLines * 2;
		vbDesc.Usage = D3D11_USAGE_DYNAMIC;
		vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = device->CreateBuffer(
			&vbDesc,
			nullptr,
			m_VertexBuffer.GetAddressOf()
		);

		if (FAILED(hr))
			return false;

		D3D11_BUFFER_DESC cbDesc = {};
		cbDesc.ByteWidth = sizeof(DebugCB);
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;


		if (FAILED(hr))
			return false;

		m_ConstantBuffer.Initialize(device, deviceContext);

		m_VertexShader.Initialize(device, Project::GetEditorShaderPath("DrawLine_v.cso").wstring().c_str(), InputElements::LineLayout, ARRAYSIZE(InputElements::LineLayout));

		m_PixelShader.Initialize(device, Project::GetEditorShaderPath("DrawLine_p.cso").wstring());


	}

	void DebugRenderer::BeginFrame()
	{
		m_LineVertices.clear();
	}

	void DebugRenderer::DrawLine(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& color)
	{
		if (m_LineVertices.size() + 2 > static_cast<size_t>(m_MaxLines) * 2)
			return;

		m_LineVertices.push_back({ a, color,0.0 });
		m_LineVertices.push_back({ b, color,0.0 });
	}
	void DebugRenderer::DrawLine(
		const XMFLOAT3& a,
		const XMFLOAT3& b,
		const XMFLOAT3& color,
		float duration)
	{
		if (m_LineVertices.size() + 2 > static_cast<size_t>(m_MaxLines) * 2)
			return;
		m_LineVertices.push_back({ a, color, duration });
		m_LineVertices.push_back({ b, color, duration });
	}

	void DebugRenderer::Update(float dt)
	{
		for (auto& line : m_LineVertices)
		{
			if (line.RemainingTime > 0.0f)
			{
				line.RemainingTime -= dt;
			}
		}

		std::erase_if(
			m_LineVertices,
			[](const auto& line)
			{
				return line.RemainingTime <= 0.0f;
			}
		);
	}
	void DebugRenderer::DrawAABB(const XMFLOAT3& min, const XMFLOAT3& max, const XMFLOAT3& color)
	{
		XMFLOAT3 p[8] =
		{
			{ min.x, min.y, min.z },
			{ max.x, min.y, min.z },
			{ max.x, max.y, min.z },
			{ min.x, max.y, min.z },

			{ min.x, min.y, max.z },
			{ max.x, min.y, max.z },
			{ max.x, max.y, max.z },
			{ min.x, max.y, max.z }
		};

		DrawLine(p[0], p[1], color);
		DrawLine(p[1], p[2], color);
		DrawLine(p[2], p[3], color);
		DrawLine(p[3], p[0], color);

		DrawLine(p[4], p[5], color);
		DrawLine(p[5], p[6], color);
		DrawLine(p[6], p[7], color);
		DrawLine(p[7], p[4], color);

		DrawLine(p[0], p[4], color);
		DrawLine(p[1], p[5], color);
		DrawLine(p[2], p[6], color);
		DrawLine(p[3], p[7], color);
	}

	void DebugRenderer::Flush(
		ID3D11DeviceContext* context,
		const XMMATRIX& viewProjection)
	{
		if (!context)
			return;

		D3D11_MAPPED_SUBRESOURCE mapped = {};

		HRESULT hr = context->Map(
			m_VertexBuffer.Get(),
			0,
			D3D11_MAP_WRITE_DISCARD,
			0,
			&mapped
		);

		if (FAILED(hr))
			return;

		memcpy(
			mapped.pData,
			m_LineVertices.data(),
			sizeof(DebugLineVertex) * m_LineVertices.size()
		);

		context->Unmap(m_VertexBuffer.Get(), 0);

		if (FAILED(hr))
			return;

		m_ConstantBuffer.data.ViewProjection = XMMatrixTranspose(viewProjection);

		m_ConstantBuffer.ApplyChanges();
		UINT stride = sizeof(DebugLineVertex);
		UINT offset = 0;

		ID3D11Buffer* vb = m_VertexBuffer.Get();
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetInputLayout(m_VertexShader.GetInputLayout());
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

		context->VSSetConstantBuffers(0, 1, m_ConstantBuffer.GetAddressOf());

		context->VSSetShader(m_VertexShader.GetShader(), nullptr, 0);
		context->PSSetShader(m_PixelShader.GetShader(), nullptr, 0);

		context->Draw(
			static_cast<UINT>(m_LineVertices.size()),
			0
		);

	}

}