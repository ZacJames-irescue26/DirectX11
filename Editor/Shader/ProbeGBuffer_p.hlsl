cbuffer ProbeCaptureCB
{
    float3 ProbePosition;
    float  FarPlane;
};

struct PSInput
{
    float4 PositionSS : SV_Position;
    float2 TexCoord : TEXCOORD;
    float3 NormalWS : NORMAL;
    float3 PositionWS : WORLD_POSITION;
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};

struct PSOutput
{
    float4 albedo : SV_Target0;
    float4 normal : SV_Target1;
};

Texture2D DiffuseTexture : TEXTURE: register(t0);
Texture2D NormalTexture : TEXTURE: register(t1);
Texture2D RoughnessTexture : TEXTURE: register(t2);
SamplerState objSamplerState : SAMPLER: register(s0);
PSOutput main(PSInput input)
{
    PSOutput o;
    
    float3 normalTS = NormalTexture.Sample(objSamplerState, input.TexCoord).rgb;

    float3 N = normalize(input.NormalWS);


    float3 baseColor = DiffuseTexture.Sample(objSamplerState, input.TexCoord).rgb;


    o.albedo = float4(baseColor, 1.0);
    o.normal = float4(N * 0.5f + 0.5f, 1.0f);


    return o;
}