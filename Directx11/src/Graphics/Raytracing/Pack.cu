
#include <cuda_runtime.h>              // core runtime API
#include <cuda_surface_types.h>        // surface object type
#include <surface_indirect_functions.h>   // <-- declares surf2DLayeredRead/Write
#include <cuda_fp16.h>
#include <cuda_runtime.h>
#include <optix.h>
#include "gdt/math/vec.h"
#include "LaunchParams.h"

using namespace gdt;
using namespace osc;

extern "C" __constant__ LaunchParams lp;   // declared once in host code
/*---------------------------------------------------------------------*/
/*  packing helper                                                     */
/*---------------------------------------------------------------------*/
__device__ __forceinline__ unsigned int packR11G11B10(vec3f c)
{
    c = max(c, vec3f(0.f));
    uint16_t rx = __half_as_ushort(__float2half_rn(c.x));
    uint16_t gy = __half_as_ushort(__float2half_rn(c.y));
    uint16_t bz = __half_as_ushort(__float2half_rn(c.z));
    return ((rx >> 5) & 0x07FF) | (((gy >> 5) & 0x07FF) << 11)
        | (((bz >> 6) & 0x03FF) << 22);
}
/*---------------------------------------------------------------------*/
/*  ray-generation : one thread = one destination texel                */
/*---------------------------------------------------------------------*/
extern "C" __global__
void __raygen__pack_texels()
{
    const unsigned flat = optixGetLaunchIndex().x;
    // only run exactly lp.texels threads
    if (flat >= lp.texels) return;

    const unsigned faceRes = 6u;
    const unsigned facesPerProbe = 6u;
    const unsigned probes = lp.probeCount;
    const unsigned tilesPerFace = faceRes * faceRes;    // 36
    const unsigned arraySize = probes * facesPerProbe;

    // decompose flat  probe, face, u, v
    const unsigned probe = flat / (facesPerProbe * tilesPerFace);
    const unsigned face = (flat / tilesPerFace) % facesPerProbe;
    const unsigned texel = flat % tilesPerFace;
    const unsigned u = texel % faceRes;
    const unsigned v = texel / faceRes;

    const unsigned layer = probe * facesPerProbe + face;
    if (probe < probes && face < facesPerProbe && layer < arraySize)
    {
        // compute the index into your linear lp.irrAccum array:
        const unsigned int idx = layer * tilesPerFace + v * faceRes + u;

        // fetch and pack
        vec4f f = lp.irrAccum[idx];
        uint32_t packed = packR11G11B10(vec3f(f.x,f.y,f.z));

        // write into the layered surface
        surf2DLayeredwrite<uint32_t>(
            packed,
            lp.irrSurf,
            int(u), int(v),
            int(layer)
        );


    }

    
}