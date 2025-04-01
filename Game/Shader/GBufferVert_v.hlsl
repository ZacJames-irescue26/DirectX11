cbuffer buffer : register(b0)
{
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
    float4x4 worldInvTransposeMatrix;
}; 

struct VS_INPUT
{
    float3 inPos : POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inNormal : NORMAL;
    float3 inTangent : TANGENT;
    float3 inBiTangent : BITANGENT;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
    float3 outNormal : NORMAL;
    float3 outTangent : TANGENT;
    float3 outBiTangent : BITANGENT;
    float3 outWorldPos : WORLD_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldPos = mul(float4(input.inPos, 1.0f), worldMatrix);
    output.outPosition = mul(worldPos, wvpMatrix);
    output.outTexCoord = input.inTexCoord;
    output.outWorldPos = worldPos.xyz;
    
    float3x3 normalMatrix = (float3x3) worldInvTransposeMatrix;

    output.outNormal = normalize(mul(input.inNormal, normalMatrix));
    output.outTangent = normalize(mul(input.inTangent, normalMatrix));
    output.outBiTangent = normalize(mul(input.inBiTangent, normalMatrix));

    return output;
}