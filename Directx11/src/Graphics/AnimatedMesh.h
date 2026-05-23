#pragma once
#include "Texture.h"
#include "BoneData.h"
#include "src/Graphics/VertexBuffer.h"
#include "IndexBuffer.h"
#include "src/Graphics/Vertex.h"
#include "AnimationInfo.h"
namespace Engine
{
	inline DirectX::XMMATRIX AiMatrixToXMMATRIX(const aiMatrix4x4& m)
	{
		return DirectX::XMMATRIX(
			m.a1, m.a2, m.a3, m.a4,
			m.b1, m.b2, m.b3, m.b4,
			m.c1, m.c2, m.c3, m.c4,
			m.d1, m.d2, m.d3, m.d4
		);


	}
	class AnimatedMesh
	{
	public:

		AnimatedMesh(ID3D11Device* device, ID3D11DeviceContext* deviceContext, std::vector<AnimatedVertex>& Vertices, std::vector<DWORD>& indices, std::vector<Texture> tex, const XMMATRIX& Matrix);
		void DrawJustMesh();
		void Draw();
		const XMMATRIX& GetTransformMatrix();
		std::vector<AnimatedVertex> m_vertices;
		std::vector<DWORD> m_indices;
		std::vector<Texture> textures;
	private:
		VertexBuffer<AnimatedVertex> vertexbuffer;
		IndexBuffer indexbuffer;
		ID3D11DeviceContext* deviceContext;
		XMMATRIX transformMatrix;

	};
}