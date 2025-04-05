

Texture2D EquiTexture : TEXTURE : register(t0);

SamplerState HDRISamplerState : SAMPLER : register(s0);

struct FS_INPUT
{
    float4 outPosition : SV_Position;
    float3 localPos : POSITION;
};

float2 SampleSphericalMap(float3 v)
{
    const float2 invAtan = float2(0.1591, 0.3183);
    float2 uv = float2(atan2(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

struct PS_OUTPUT
{
    float4 coloroutput : SV_Target0;
};


 PS_OUTPUT main(FS_INPUT input) : SV_TARGET
{
    PS_OUTPUT output;
    float2 uv = SampleSphericalMap(normalize(input.localPos)); // make sure to normalize localPos
    float3 color = EquiTexture.Sample(HDRISamplerState, uv).rgb;

    output.coloroutput = float4(color, 1.0);
    return output;
}
