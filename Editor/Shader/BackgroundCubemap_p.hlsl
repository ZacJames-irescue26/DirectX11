TextureCube environmentTexture : TEXTURE : register(t0);

SamplerState HDRISamplerState : SAMPLER : register(s0);

struct FS_INPUT
{
    float4 outPosition : SV_Position;
    float3 localPos : POSITION;
};

float4 main(FS_INPUT input): SV_Target
{
    float3 envColor = environmentTexture.Sample(HDRISamplerState, input.localPos).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + float3(1.0,1.0,1.0));
    envColor = pow(envColor, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));
    
    return float4(envColor, 1.0);
}
