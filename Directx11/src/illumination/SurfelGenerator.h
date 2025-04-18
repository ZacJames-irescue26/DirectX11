#pragma once
#include "Surfel.h"
#include "../Game/GameObject.h"
#include "src/Acceleration/Octree/Octree.h"
namespace Engine
{
	class SurfelGenerator
	{
	public:

		SurfelGenerator(ID3D11Device* device, ID3D11DeviceContext* devicecontext, Octree* octree,  std::vector<GameObject>& Meshes);
		SurfelGenerator(ID3D11Device* device, ID3D11DeviceContext* devicecontext, const std::string& filename);
		~SurfelGenerator();
		void SaveToFile(const std::string& filename);
		bool ReadSurfels(const std::string& filename);
		void GenerateSurfelsOnMesh(ID3D11Device* device, ID3D11DeviceContext* devicecontext, Octree* octree,  std::vector<GameObject>& Meshes);

		std::vector<Surfel*> GeneratedSurfels;
	private:
		
		int openfiletries = 0;
	};
}