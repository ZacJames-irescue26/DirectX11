#include <cuda_runtime.h>
#include <cuda_surface_types.h>
#include <cuda_fp16.h>
#include <optix_device.h>
#include "gdt/math/vec.h"
#include "LaunchParams.h"
#include <cstdint>
using namespace gdt;
using namespace osc;

extern "C" __constant__ LaunchParams lp;

__device__ __forceinline__ vec3f fibonacciDir(uint32_t i)
{
    const float k = float(i) + 0.5f, n = 256.f, z = 1.f - 2.f * k / n;
    const float r = sqrtf(max(0.f, 1.f - z * z)), phi = k * 2.39996323f;
    return vec3f(cosf(phi) * r, sinf(phi) * r, z);
}

struct FaceUV { unsigned int face, u, v; };
__device__ __forceinline__ FaceUV dirToCube(vec3f d)
{
    vec3f a = abs(d); unsigned int f; float sc, tc, ma;
    if (a.x >= a.y && a.x >= a.z) { f = d.x > 0 ? 0 : 1; ma = a.x; sc = d.z; tc = d.y * (d.x > 0 ? 1 : -1); }
    else if (a.y >= a.x && a.y >= a.z) { f = d.y > 0 ? 2 : 3; ma = a.y; sc = d.x; tc = d.z * (d.y > 0 ? -1 : 1); }
    else { f = d.z > 0 ? 4 : 5; ma = a.z; sc = d.z > 0 ? -d.x : d.x; tc = d.y; }
    unsigned int u = min(unsigned int((sc / ma * 0.5f + 0.5f) * 6.f), 5u);
    unsigned int v = min(unsigned int((tc / ma * 0.5f + 0.5f) * 6.f), 5u);
    return { f,u,v };
}

/* R11G11B10 packing  (returns 32-bit uint) */
__device__ __forceinline__ unsigned int packR11G11B10(vec3f c)
{
    uint16_t rx = __half_as_ushort(__float2half_rn(c.x)); // 5 exponent, 6 mant.
    uint16_t gy = __half_as_ushort(__float2half_rn(c.y));
    uint16_t bz = __half_as_ushort(__float2half_rn(c.z)); // only 5 mantissa bits
    return ((rx >> 5) & 0x7FF) | (((gy >> 5) & 0x7FF) << 11)
        | (((bz >> 6) & 0x3FF) << 22);
}

/*???????????????? payload helpers ?????????*/
struct PayloadRGB { vec3f L; };
static __device__ __forceinline__ void setPayload(const vec3f& L) {
    optixSetPayload_0(__float_as_uint(L.x));
    optixSetPayload_1(__float_as_uint(L.y));
    optixSetPayload_2(__float_as_uint(L.z));
}

/*???????????????? miss – simple sky ???????*/
extern "C" __global__ void __miss__radiance()
{
    setPayload(vec3f(0.03f, 0.04f, 0.05f));
}

/*???????????????? closest-hit radiance ????*/
extern "C" __global__ void __closesthit__radiance()
{
    /* your real shading here – for now constant white */
    setPayload(vec3f(1.f));
}

/*???????????????? ray-gen 0 : accumulate ??*/
extern "C" __global__
void __raygen__ddgi_accum()
{
    const unsigned int rayID = optixGetLaunchIndex().x;   // 0..255
    const unsigned int probeID = optixGetLaunchIndex().y;

    unsigned int j = (rayID + 47u * lp.frameID) & 255u;
    vec3f dir = fibonacciDir(j);
    vec3f org = lp.probePos[probeID];

    /* payload registers */
    unsigned int p0 = 0, p1 = 0, p2 = 0;
    optixTrace(lp.tlas, org, dir,
        1e-3f, 1e20f, 0.0f,
        0xFF, OPTIX_RAY_FLAG_NONE,
        0, 1, 0,
        p0, p1, p2);

    vec3f L = { __uint_as_float(p0), __uint_as_float(p1), __uint_as_float(p2) };

    float  w = fabsf(dir.z) * (1.f / M_PI) / 256.f;
    vec3f  dE = L * w;

    FaceUV f = dirToCube(dir);
    unsigned int   idx = (probeID * 6u + f.face) * 36u + f.v * 6u + f.u;

    atomicAdd(&lp.irrAccum[idx].x, dE.x);
    atomicAdd(&lp.irrAccum[idx].y, dE.y);
    atomicAdd(&lp.irrAccum[idx].z, dE.z);
    atomicAdd(&lp.irrAccum[idx].w, 1.0f);
}