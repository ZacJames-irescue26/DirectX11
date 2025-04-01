
cbuffer ViewProj : register(b0)
{
    float4x4 proj;
    float4x4 view;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_Position;
    float3 localPos : POSITION;
};

VS_OUTPUT main( float3 pos : POSITION )
{
    VS_OUTPUT output;
    output.localPos = pos;
    float4x4 viewproj = mul(proj, view);
    output.outPosition = mul(viewproj, float4(pos, 1.0f));
    return output;
}