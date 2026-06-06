#include "../../include/GPUSurfel.hlsli"

StructuredBuffer<GPUSurfel> Surfels : register(t0);
StructuredBuffer<float4> ProbePositions : register(t1); // xyz position, w radius
Texture2D<float> ShadowMap : register(t2);
SamplerComparisonState ShadowSampler : register(s0);

RWStructuredBuffer<uint4> ProbeAccum : register(u0);

cbuffer LightCB : register(b0)
{
    float3 LightDirection;
    float  LightIntensity;

    float3 LightColor;
    uint   SurfelCount;

    uint   ProbeCount;
    float  RadianceScale;
    float  ProbeInfluenceRadius;
    float  Padding0;
    float4x4 LightViewProjection;
};
float SampleDirectionalShadowReversedZ(
    float3 worldPos,
    float3 normal,
    float3 lightDir)
{
    float4 lightClip = mul(float4(worldPos, 1.0f), LightViewProjection);

    float3 ndc = lightClip.xyz / lightClip.w;

    float2 uv;
    uv.x = ndc.x * 0.5f + 0.5f;
    uv.y = -ndc.y * 0.5f + 0.5f;

    float depth = ndc.z;

    if (uv.x < 0.0f || uv.x > 1.0f || uv.y < 0.0f || uv.y > 1.0f)
        return 0.0f;
    float ShadowBias = 0.001f;
    // Reversed-Z bias:
    // For normal-Z you usually subtract bias.
    // For reversed-Z, you usually add bias.
    float NoL = saturate(dot(normal, normalize(-lightDir)));
    float slopeBias = ShadowBias * (1.0f - NoL);

    float compareDepth = depth + ShadowBias + slopeBias;

    return ShadowMap.SampleCmpLevelZero(
        ShadowSampler,
        uv,
        compareDepth
    );
}
void AtomicAddRGB(uint index, float3 value)
{
    value = max(value, 0.0f);

    // Debug clamp to avoid huge white speckles.
    value = min(value, 1.0f.xxx);

    uint3 packed = (uint3)min(value * RadianceScale, 4000000000.0f);

    InterlockedAdd(ProbeAccum[index].x, packed.x);
    InterlockedAdd(ProbeAccum[index].y, packed.y);
    InterlockedAdd(ProbeAccum[index].z, packed.z);
    InterlockedAdd(ProbeAccum[index].w, 1);
}

void AddDirectionalToAmbientCube(
    uint probeIndex,
    float3 direction,
    float3 radiance)
{
    direction = normalize(direction);

    float3 d2 = direction * direction;

    if (direction.x > 0.0f)
        AtomicAddRGB(probeIndex * 6 + 0, radiance * d2.x);
    else
        AtomicAddRGB(probeIndex * 6 + 1, radiance * d2.x);

    if (direction.y > 0.0f)
        AtomicAddRGB(probeIndex * 6 + 2, radiance * d2.y);
    else
        AtomicAddRGB(probeIndex * 6 + 3, radiance * d2.y);

    if (direction.z > 0.0f)
        AtomicAddRGB(probeIndex * 6 + 4, radiance * d2.z);
    else
        AtomicAddRGB(probeIndex * 6 + 5, radiance * d2.z);
}

[numthreads(128, 1, 1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint surfelIndex = id.x;

    if (surfelIndex >= SurfelCount)
        return;

    GPUSurfel s = Surfels[surfelIndex];

    float3 N = normalize(s.Normal);
    float3 L = normalize(-LightDirection);

    float NoL = saturate(dot(N, L));

    if (NoL <= 0.0f)
        return;

    float shadow = SampleDirectionalShadowReversedZ(
        s.Position,
        N,
        LightDirection
    );

    if (shadow <= 0.001f)
        return;

    float area = clamp(s.Area, 0.001f, 0.03f);

    float3 bounce =
        LightColor *
        LightIntensity *
        NoL *
        shadow *
        s.Albedo *
        area;

    float radius = ProbeInfluenceRadius;
    float radius2 = radius * radius;

    // Brute force all probes for now.
    // Later replace this with surfel-to-probe links or a probe grid.
    for (uint probeIndex = 0; probeIndex < ProbeCount; ++probeIndex)
    {
        float3 probePos = ProbePositions[probeIndex].xyz;

        float3 toProbe = probePos - s.Position;
        float dist2 = dot(toProbe, toProbe);

        if (dist2 > radius2)
            continue;

        float dist = sqrt(max(dist2, 0.0001f));
        float3 dirToProbe = toProbe / dist;

        // Surfel emits mostly along its normal.
        float facing = saturate(dot(N, dirToProbe));

        if (facing <= 0.0f)
            continue;

        float falloff = saturate(1.0f - dist / radius);

        float weight =
            facing *
            falloff *
            falloff /
            max(dist2, 0.25f);

        float3 contribution = bounce * weight;

        // Direction light is coming FROM at the probe.
        float3 incomingDir = normalize(s.Position - probePos);

        AddDirectionalToAmbientCube(
            probeIndex,
            incomingDir,
            contribution
        );
    }
}