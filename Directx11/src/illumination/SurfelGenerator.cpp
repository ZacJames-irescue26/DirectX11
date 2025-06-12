#include "pch.h"
#include "SurfelGenerator.h"

namespace Engine
{
	XMVECTOR randomBarycentricCoords() {
		float u = static_cast<float>(rand()) / RAND_MAX;
		float v = static_cast<float>(rand()) / RAND_MAX;
		if (u + v > 1.0f) {
			u = 1.0f - u;
			v = 1.0f - v;
		}
		auto coords = XMFLOAT3(u, v, 1.0f - u - v);
		return XMLoadFloat3(&coords);
	}

	// Function to sample a pixel at (u, v)
	XMFLOAT4 sampleTexture(const std::vector<BYTE>& textureData, int width, int height, float u, float v) {
		// Convert normalized UVs (0 to 1) to pixel indices
		int x = static_cast<int>(u * (width - 1));
		int y = static_cast<int>(v * (height - 1));

		// Ensure the coordinates are within bounds
		x = std::clamp(x, 0, width - 1);
		y = std::clamp(y, 0, height - 1);

		// Compute the index in the texture data array
		int index = (y * width + x) * 4; // Assuming 4 channels (RGBA)

		// Extract the RGBA values and normalize to [0, 1]
		float r = textureData[index + 0] / 255.0f;
		float g = textureData[index + 1] / 255.0f;
		float b = textureData[index + 2] / 255.0f;
		float a = textureData[index + 3] / 255.0f;

		return XMFLOAT4(r, g, b, a);
	}
	//bool DoSpheresOverlap(const glm::vec3& sphere1, const glm::vec3& sphere2, float radius) {
	//	float dx = sphere2.x - sphere1.x;
	//	float dy = sphere2.y - sphere1.y;
	//	float dz = sphere2.z - sphere1.z;
	//	float distanceSquared = dx * dx + dy * dy + dz * dz;
	//	float radiiSum = radius + radius;
	//	return distanceSquared <= radiiSum * radiiSum;
	//}

	std::vector<BYTE> retreiveTexture(ID3D11Device* device, ID3D11DeviceContext* devicecontext, ID3D11Texture2D* pSourceTexture, int& outWidth, int& outHeight)
	{
		// 1. Create a staging texture
		D3D11_TEXTURE2D_DESC sourceDesc;
		pSourceTexture->GetDesc(&sourceDesc);
		ID3D11Texture2D* pStagingTexture;
		D3D11_TEXTURE2D_DESC stagingDesc = {};
		stagingDesc.Width = sourceDesc.Width;
		stagingDesc.Height = sourceDesc.Height;
		stagingDesc.MipLevels = sourceDesc.MipLevels;
		stagingDesc.ArraySize = 1;
		stagingDesc.Format = sourceDesc.Format;
		stagingDesc.SampleDesc.Count = 1;
		stagingDesc.SampleDesc.Quality = 0;
		stagingDesc.Usage = D3D11_USAGE_STAGING;
		stagingDesc.BindFlags = 0;
		stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		stagingDesc.MiscFlags = 0;
		HRESULT hr = device->CreateTexture2D(&stagingDesc, NULL, &pStagingTexture);

		// 2. Copy the texture data to the staging texture
		D3D11_BOX srcBox;
		srcBox.left = 0;
		srcBox.right = sourceDesc.Width;
		srcBox.top = 0;
		srcBox.bottom = sourceDesc.Height;
		srcBox.front = 0;
		srcBox.back = 1;
		devicecontext->CopySubresourceRegion(pStagingTexture, 0, 0, 0, 0, pSourceTexture, 0, &srcBox);

		// 3. Map the staging texture
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		hr = devicecontext->Map(pStagingTexture, 0, D3D11_MAP_READ, 0, &mappedResource);
		if (FAILED(hr)) {
			// Handle error
		}

		// 4. Access the albedo data
		// Assuming the texture format is DXGI_FORMAT_R8G8B8A8_UNORM (8-bit RGBA)
		outWidth = sourceDesc.Width;
		outHeight = sourceDesc.Height;


		std::vector<BYTE> OutTex;
		size_t rowPitch = sourceDesc.Width * 4; // Assuming DXGI_FORMAT_R8G8B8A8_UNORM
		OutTex.resize(outWidth * outHeight * 4);
		BYTE* dstPtr = OutTex.data();
		BYTE* srcPtr = static_cast<BYTE*>(mappedResource.pData);
		// Copy row by row (handle RowPitch != Width * 4)
		for (UINT row = 0; row < sourceDesc.Height; ++row)
		{
			memcpy(dstPtr + row * rowPitch, srcPtr + row * mappedResource.RowPitch, rowPitch);
		}


		// 5. Unmap the staging texture
		devicecontext->Unmap(pStagingTexture, 0);

		// Release the staging texture
		pStagingTexture->Release();

		return OutTex;
	}

	SurfelGenerator::SurfelGenerator(ID3D11Device* device, ID3D11DeviceContext* devicecontext, Octree* octree,  std::vector<GameObject>& Meshes)
	{

		GenerateSurfelsOnMesh(device, devicecontext, octree, Meshes);
	}

	SurfelGenerator::SurfelGenerator(ID3D11Device* device, ID3D11DeviceContext* devicecontext, const std::string& filename)
	{
		if (!ReadSurfels(filename))
		{
			std::cout << std::format("Failed to open file or load surfels \n");
		}
	}

	SurfelGenerator::~SurfelGenerator()
	{
		for (int i = 0; i < GeneratedSurfels.size(); i++)
		{
			//delete GeneratedSurfels[i];
		}
		GeneratedSurfels.empty();
	}

	void SurfelGenerator::SaveToFile(const std::string& filename)
	{
		//// open file stream for writing and reading
		//if (openfiletries > 5)
		//{
		//	std::cout << "Cant create file or open file";
		//	return;
		//}
		//std::fstream s{ filename, s.binary | s.trunc | s.out };
		//if (!s.is_open()) {
		//	std::cout << "cannot open " << filename << " for writing\n";
		//}
		//bitsery::Serializer<bitsery::OutputBufferedStreamAdapter> ser{ s };
		//for (const auto& surfel : GeneratedSurfels)
		//{
		//	ser.object(surfel);  
		//}
		//ser.adapter().flush();
		//s.close();
	}

	bool SurfelGenerator::ReadSurfels(const std::string& filename)
	{
		return false;
	}

	void SurfelGenerator::GenerateSurfelsOnMesh(ID3D11Device* device, ID3D11DeviceContext* devicecontext, Octree* octree,  std::vector<GameObject>& Meshes)
	{
		float surfelDensityFactor = 1;
		std::vector<Triangle> triangles;
		for ( auto& obj : Meshes)
		{
			XMMATRIX model = obj.worldMatrix;

			XMMATRIX normalmatrix =XMMatrixTranspose(XMMatrixInverse(nullptr, model));
			for (auto& meshes : obj.GetModel().GetMeshes())
			{

				// Get the texture's width, height, and format (ensure these match your texture settings)
				int width, height;
				std::vector<BYTE> textureData = retreiveTexture(device,devicecontext, meshes.textures[0].GetRawTexture(), width, height);
				
				// Create a buffer to hold the texture data
				 // Assuming 4 channels (RGBA)


				uint32_t vertexCount = meshes.vertices.size();
				vertexCount -= meshes.vertices.size() % 3;
				for (int i = 0; i < vertexCount; i += 3)
				{
					Triangle tri;

					tri.p0 = XMVector3Transform(XMLoadFloat3(&meshes.vertices[i].pos), model);
					tri.p1 = XMVector3Transform(XMLoadFloat3(&meshes.vertices[i+1].pos), model);
					tri.p2 = XMVector3Transform(XMLoadFloat3(&meshes.vertices[i+2].pos), model);

					float area = tri.area();
					
					int SurfelDensity = static_cast<int>(area * surfelDensityFactor);

					// There are very likely to be degenerate triangles which we don't want
					if (area <= 1e-06) {
						continue;
					}

					tri.n0 = XMVector3TransformNormal(XMLoadFloat3(&meshes.vertices[i].normal), normalmatrix);
					tri.n1 = XMVector3TransformNormal(XMLoadFloat3(&meshes.vertices[i + 1].normal), normalmatrix);
					tri.n2 = XMVector3TransformNormal(XMLoadFloat3(&meshes.vertices[i + 2].normal), normalmatrix);

					tri.t0 = XMLoadFloat2(&meshes.vertices[i].texCoord);
					tri.t1 = XMLoadFloat2(&meshes.vertices[i + 1].texCoord);
					tri.t2 = XMLoadFloat2(&meshes.vertices[i + 2].texCoord);


					for (int i = 0; i < 1; i++)
					{
						bool isoccupied = false;
						XMVECTOR barycentric = randomBarycentricCoords();
						XMVECTOR Position = tri.p0 *  XMVectorGetX(barycentric) + tri.p1 * XMVectorGetY(barycentric) + tri.p2 * XMVectorGetZ(barycentric);
						XMVECTOR Normal = tri.n0 * XMVectorGetX(barycentric) + tri.n1 * XMVectorGetY(barycentric) + tri.n2 * XMVectorGetZ(barycentric);
						XMVECTOR UV = tri.t0 * XMVectorGetX(barycentric) + tri.t1 * XMVectorGetY(barycentric) + tri.t2 * XMVectorGetZ(barycentric);
						XMFLOAT4 albedo = sampleTexture(textureData, width, height, XMVectorGetX(UV), XMVectorGetY(UV));

						XMFLOAT3 pos;
						XMStoreFloat3(&pos, Position);
						XMFLOAT3 norm;
						XMStoreFloat3(&norm, Normal);

						Surfel* candidate = new Surfel(pos, norm, albedo, 100.0);
						std::array<OctreeNode*, 7> ContainingNode = { octree->FindSmallestAABB(octree->Root(), candidate->position),
						octree->FindSmallestAABB(octree->Root(), { candidate->position.x + 10, candidate->position.y, candidate->position.z }),
						octree->FindSmallestAABB(octree->Root(), { candidate->position.x - 10, candidate->position.y, candidate->position.z }),
						octree->FindSmallestAABB(octree->Root(), { candidate->position.x , candidate->position.y + 10, candidate->position.z }),
						octree->FindSmallestAABB(octree->Root(), { candidate->position.x , candidate->position.y - 10, candidate->position.z }),
						octree->FindSmallestAABB(octree->Root(), { candidate->position.x, candidate->position.y, candidate->position.z + 10 }),
						octree->FindSmallestAABB(octree->Root(), { candidate->position.x, candidate->position.y, candidate->position.z - 10 }),
						};

						for (auto node : ContainingNode)
						{
							for (auto surfel : node->surfels)
							{
								if (surfel->aabb.OverlappingwithSphere(candidate->position, candidate->radius))
								{
									isoccupied = true;
									break;

								}
							}

						}
						if (isoccupied == false)
						{
							ContainingNode[0]->surfels.push_back(candidate);
							GeneratedSurfels.push_back(candidate);
							std::cout << "Adding surfel \n";
						}

						candidate = nullptr;
					}



				}
			}
		}

	}




}