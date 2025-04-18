cbuffer LightMatrix : register(b1)
{
    float4x4 lightSpaceMatrix;
};

cbuffer ObjectMatrix : register(b0)
{
    float4x4 modelMatrix;
    
};

struct VS_INPUT
{
    float3 pos : POSITION;
};

struct VS_OUTPUT
{
    float4 pos : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 worldPos = mul(float4(input.pos, 1.0f), modelMatrix);
    output.pos = mul(worldPos, lightSpaceMatrix);
    return output;
}