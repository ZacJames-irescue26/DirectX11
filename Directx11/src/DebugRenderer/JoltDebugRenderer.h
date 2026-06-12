#pragma once
#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>
#include "pch.h"
#include "DebugRenderer.h"
namespace Engine
{
	class JoltDebugRenderer final : public JPH::DebugRenderer
	{
	private:
		class TriangleBatch final
			: public JPH::RefTargetVirtual,
			public JPH::RefTarget<TriangleBatch>
		{
		public:
			JPH_OVERRIDE_NEW_DELETE

				std::vector<JPH::DebugRenderer::Vertex> Vertices;
			std::vector<JPH::uint32> Indices;

			void AddRef() override
			{
				JPH::RefTarget<TriangleBatch>::AddRef();
			}

			void Release() override
			{
				JPH::RefTarget<TriangleBatch>::Release();
			}
		};

	public:
		JoltDebugRenderer()
		{
			Initialize();
			JPH::DebugRenderer::sInstance = this;
		}
		~JoltDebugRenderer() override{
			if (JPH::DebugRenderer::sInstance == this)
			{
				JPH::DebugRenderer::sInstance = nullptr;
			}
		
		}
		
		void DrawLine(RVec3Arg inFrom, RVec3Arg inTo, ColorArg inColor) override
		{
			XMFLOAT3 a(
				static_cast<float>(inFrom.GetX()),
				static_cast<float>(inFrom.GetY()),
				static_cast<float>(inFrom.GetZ())
			);

			XMFLOAT3 b(
				static_cast<float>(inTo.GetX()),
				static_cast<float>(inTo.GetY()),
				static_cast<float>(inTo.GetZ())
			);

			XMFLOAT3 color(
				inColor.r / 255.0f,
				inColor.g / 255.0f,
				inColor.b / 255.0f
			);

			Engine::DebugRenderer::Get()->DrawLine(a, b, color);
		}

		void DrawTriangle(RVec3Arg inV1, RVec3Arg inV2, RVec3Arg inV3, ColorArg inColor) override
		{
			XMFLOAT3 a(inV1.GetX(), inV1.GetY(), inV1.GetZ());
			XMFLOAT3 b(inV2.GetX(), inV2.GetY(), inV2.GetZ());
			XMFLOAT3 c(inV3.GetX(), inV3.GetY(), inV3.GetZ());

			XMFLOAT3 color(
				inColor.r / 255.0f,
				inColor.g / 255.0f,
				inColor.b / 255.0f
			);

			Engine::DebugRenderer::Get()->DrawLine(a, b, color);
			Engine::DebugRenderer::Get()->DrawLine(b, c, color);
			Engine::DebugRenderer::Get()->DrawLine(c, a, color);
		}

		Batch CreateTriangleBatch(const Triangle* inTriangles, int inTriangleCount) override
		{
			if (!inTriangles || inTriangleCount <= 0)
				return nullptr;

			TriangleBatch* batch = new TriangleBatch();

			batch->Vertices.reserve(
				static_cast<size_t>(inTriangleCount) * 3
			);

			batch->Indices.reserve(
				static_cast<size_t>(inTriangleCount) * 3
			);

			for (int i = 0; i < inTriangleCount; ++i)
			{
				const Triangle& triangle = inTriangles[i];

				const JPH::uint32 baseIndex =
					static_cast<JPH::uint32>(
						batch->Vertices.size()
						);

				batch->Vertices.push_back(triangle.mV[0]);
				batch->Vertices.push_back(triangle.mV[1]);
				batch->Vertices.push_back(triangle.mV[2]);

				batch->Indices.push_back(baseIndex + 0);
				batch->Indices.push_back(baseIndex + 1);
				batch->Indices.push_back(baseIndex + 2);
			}

			return batch;
		}

		Batch CreateTriangleBatch(const Vertex* inVertices, int inVertexCount, const uint32* inIndices, int inIndexCount) override
		{
			if (!inVertices || inVertexCount <= 0)
				return nullptr;

			TriangleBatch* batch = new TriangleBatch();

			batch->Vertices.assign(
				inVertices,
				inVertices + inVertexCount
			);

			if (inIndices && inIndexCount > 0)
			{
				batch->Indices.assign(
					inIndices,
					inIndices + inIndexCount
				);
			}
			else
			{
				batch->Indices.resize(
					static_cast<size_t>(inVertexCount)
				);

				for (int i = 0; i < inVertexCount; ++i)
				{
					batch->Indices[i] =
						static_cast<JPH::uint32>(i);
				}
			}

			return batch;
		}

		void DrawGeometry(RMat44Arg inModelMatrix, const AABox& inWorldSpaceBounds, float inLODScaleSq, ColorArg inModelColor, const GeometryRef& inGeometry, ECullMode inCullMode = ECullMode::CullBackFace, ECastShadow inCastShadow = ECastShadow::On, EDrawMode inDrawMode = EDrawMode::Solid) override
		{

			auto* debugRenderer = Engine::DebugRenderer::Get();

			if (!debugRenderer || inGeometry == nullptr)
				return;

			// Only render collider wireframes.
			if (inDrawMode != EDrawMode::Wireframe)
				return;

			if (inGeometry->mLODs.empty())
				return;

			const LOD* selectedLOD = &inGeometry->mLODs[0];

			// Optional: choose a lower LOD depending on distance.
			for (const LOD& lod : inGeometry->mLODs)
			{
				if (lod.mDistance * lod.mDistance <= inLODScaleSq)
					selectedLOD = &lod;
				else
					break;
			}

			if (selectedLOD->mTriangleBatch == nullptr)
				return;

			const TriangleBatch* batch =
				static_cast<const TriangleBatch*>(
					selectedLOD->mTriangleBatch.GetPtr()
					);

			if (!batch)
				return;

			const XMFLOAT3 color =
			{
				inModelColor.r / 255.0f,
				inModelColor.g / 255.0f,
				inModelColor.b / 255.0f
			};

			const auto& verticesbatch = batch->Vertices;
			const auto& indices = batch->Indices;

			for (size_t i = 0; i + 2 < indices.size(); i += 3)
			{
				const JPH::uint32 i0 = indices[i + 0];
				const JPH::uint32 i1 = indices[i + 1];
				const JPH::uint32 i2 = indices[i + 2];

				if (i0 >= verticesbatch.size() ||
					i1 >= verticesbatch.size() ||
					i2 >= verticesbatch.size())
				{
					continue;
				}

				const JPH::Vec3 local0 = (JPH::Vec3)verticesbatch[i0].mPosition;
				const JPH::Vec3 local1 = (JPH::Vec3)verticesbatch[i1].mPosition;
				const JPH::Vec3 local2 = (JPH::Vec3)verticesbatch[i2].mPosition;

				const JPH::RVec3 world0 =
					inModelMatrix * local0;

				const JPH::RVec3 world1 =
					inModelMatrix * local1;

				const JPH::RVec3 world2 =
					inModelMatrix * local2;

				const XMFLOAT3 a =
				{
					static_cast<float>(world0.GetX()),
					static_cast<float>(world0.GetY()),
					static_cast<float>(world0.GetZ())
				};

				const XMFLOAT3 b =
				{
					static_cast<float>(world1.GetX()),
					static_cast<float>(world1.GetY()),
					static_cast<float>(world1.GetZ())
				};

				const XMFLOAT3 c =
				{
					static_cast<float>(world2.GetX()),
					static_cast<float>(world2.GetY()),
					static_cast<float>(world2.GetZ())
				};

				debugRenderer->DrawLine(a, b, color);
				debugRenderer->DrawLine(b, c, color);
				debugRenderer->DrawLine(c, a, color);
			} 

			for (size_t i = 0; i + 2 < indices.size(); i += 3)
			{
				const JPH::uint32 i0 = indices[i + 0];
				const JPH::uint32 i1 = indices[i + 1];
				const JPH::uint32 i2 = indices[i + 2];

				if (i0 >= verticesbatch.size() ||
					i1 >= verticesbatch.size() ||
					i2 >= verticesbatch.size())
				{
					continue;
				}

				const JPH::Vec3 local0 = (JPH::Vec3)verticesbatch[i0].mPosition;
				const JPH::Vec3 local1 = (JPH::Vec3)verticesbatch[i1].mPosition;
				const JPH::Vec3 local2 = (JPH::Vec3)verticesbatch[i2].mPosition;

				const JPH::RVec3 world0 =
					inModelMatrix * local0;

				const JPH::RVec3 world1 =
					inModelMatrix * local1;

				const JPH::RVec3 world2 =
					inModelMatrix * local2;

				const XMFLOAT3 a =
				{
					static_cast<float>(world0.GetX()),
					static_cast<float>(world0.GetY()),
					static_cast<float>(world0.GetZ())
				};

				const XMFLOAT3 b =
				{
					static_cast<float>(world1.GetX()),
					static_cast<float>(world1.GetY()),
					static_cast<float>(world1.GetZ())
				};

				const XMFLOAT3 c =
				{
					static_cast<float>(world2.GetX()),
					static_cast<float>(world2.GetY()),
					static_cast<float>(world2.GetZ())
				};

				debugRenderer->DrawLine(a, b, color);
				debugRenderer->DrawLine(b, c, color);
				debugRenderer->DrawLine(c, a, color);
			}

		}

		void DrawText3D(RVec3Arg inPosition, const string_view& inString, ColorArg inColor = JPH::Color::sWhite, float inHeight = 0.5f) override
		{
			return;
		}

		
	};
}