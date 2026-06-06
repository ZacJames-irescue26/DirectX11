
#include "../../include/GPUProbe.hlsli"




StructuredBuffer<uint4> ProbeAccum : register(t0);





RWStructuredBuffer<GPUProbe> Probes : register(u0);

cbuffer NormalizeCB : register(b0)
{
    uint ProbeCount;
    float InvRadianceScale;
    float2 Padding;
};

[numthreads(64, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint probeIndex = id.x;

    if (probeIndex >= ProbeCount)
        return;

    GPUProbe p = Probes[probeIndex];

    uint4 a0 = ProbeAccum[probeIndex * 6 + 0];
    uint4 a1 = ProbeAccum[probeIndex * 6 + 1];
    uint4 a2 = ProbeAccum[probeIndex * 6 + 2];
    uint4 a3 = ProbeAccum[probeIndex * 6 + 3];
    uint4 a4 = ProbeAccum[probeIndex * 6 + 4];
    uint4 a5 = ProbeAccum[probeIndex * 6 + 5];

    p.AmbientCube0 = float3(a0.x, a0.y, a0.z) * InvRadianceScale;
    p.AmbientCube1 = float3(a1.x, a1.y, a1.z) * InvRadianceScale;
    p.AmbientCube2 = float3(a2.x, a2.y, a2.z) * InvRadianceScale;
    p.AmbientCube3 = float3(a3.x, a3.y, a3.z) * InvRadianceScale;
    p.AmbientCube4 = float3(a4.x, a4.y, a4.z) * InvRadianceScale;
    p.AmbientCube5 = float3(a5.x, a5.y, a5.z) * InvRadianceScale;

    Probes[probeIndex] = p;
}