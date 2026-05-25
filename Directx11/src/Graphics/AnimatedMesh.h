#pragma once
#include "Texture.h"
#include "BoneData.h"
#include "src/Graphics/VertexBuffer.h"
#include "IndexBuffer.h"
#include "src/Graphics/Vertex.h"
#include "AnimationInfo.h"
namespace Engine
{
	inline XMMATRIX AiMatrixToXMMATRIX(const aiMatrix4x4& m)
	{
		return XMMATRIX(
			m.a1, m.b1, m.c1, m.d1,
			m.a2, m.b2, m.c2, m.d2,
			m.a3, m.b3, m.c3, m.d3,
			m.a4, m.b4, m.c4, m.d4
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
		ID3D11Device* device;
		VertexBuffer<AnimatedVertex> vertexbuffer;
		IndexBuffer indexbuffer;
		ID3D11DeviceContext* deviceContext;
		XMMATRIX transformMatrix;

	};
}