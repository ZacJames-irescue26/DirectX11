#pragma once
#include "DebugLineVertex.h"
#include "src/Graphics/Shader.h"
#include "src/Graphics/ConstantBuffer.h"

namespace Engine
{
	class DebugRenderer
	{
	public:
		bool Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceCOntext, uint32_t maxLines = 65536);
		static void Set(DebugRenderer* renderer)
		{
			s_Instance = renderer;
		}

		static DebugRenderer* Get()
		{
			return s_Instance;
		}
		void BeginFrame();
		void DrawLine(
			const XMFLOAT3& a,
			const XMFLOAT3& b,
			const XMFLOAT3& color);

		void DrawLine(const XMFLOAT3& a, const XMFLOAT3& b, const XMFLOAT3& color, float duration);

		void Update(float dt);
		void DrawAABB(
			const XMFLOAT3& min,
			const XMFLOAT3& max,
			const XMFLOAT3& color);

		void Flush(
			ID3D11DeviceContext* context,
			const XMMATRIX& viewProjection);

	private:
		struct DebugCB
		{
			XMMATRIX ViewProjection;
		};

	private:

		inline static DebugRenderer* s_Instance = nullptr;
		std::vector<DebugLineVertex> m_LineVertices;
		std::vector<DebugLineVertex> m_PermaLineVertices;
		uint32_t m_MaxLines = 0;

		Microsoft::WRL::ComPtr<ID3D11Buffer> m_VertexBuffer;
		ConstantBuffer<DebugCB> m_ConstantBuffer;

		VertexShader  m_VertexShader;
		PixelShader m_PixelShader;
	};
}