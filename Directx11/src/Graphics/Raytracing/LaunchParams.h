#pragma once
#include "cuda_d3d11_interop.h"
#include "optix_types.h"
#include "gdt/gdt.h"
#include "gdt/math/vec.h"
#include "optix.h" 
#include <texture_types.h>
namespace osc {

	using namespace gdt;
	// for this simple example, we have a single ray type
	enum { RADIANCE_RAY_TYPE = 0, SHADOW_RAY_TYPE, RAY_TYPE_COUNT};

	struct TriangleMeshSBTData {
		vec3f  color;
		vec3f* vertex;
		vec3f* normal;
		vec2f* texcoord;
		vec3i* index;
		bool                hasTexture;
		cudaTextureObject_t texture;
		
	};


	struct alignas(16) LaunchParams
	{
		OptixTraversableHandle tlas;
		float Packing;
		float packing1;
		vec4f* irrAccum; 
		cudaSurfaceObject_t   irrSurf;  
		float Packing2;
		int curSlice = 0;        
		const vec3f* probePos;
		float Packing3;
		uint32_t              probeCount = 0;
		uint32_t              frameID = 0;
		uint32_t              texels = 0;    // = probeCount * 6 * 36
		
	};

	/*struct LaunchParams
	{
		struct {
			uint32_t* colorBuffer;
			vec2i     size;
			int       accumID{ 0 };
		} frame;

		struct {
			vec3f position;
			vec3f direction;
			vec3f horizontal;
			vec3f vertical;
		} camera;

		struct {
			vec3f origin, du, dv, power;
		} light;

		OptixTraversableHandle traversable;
	};*/

} // ::osc