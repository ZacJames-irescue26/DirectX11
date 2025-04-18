
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
    float4x4 viewproj = mul(view, proj);
    output.outPosition = mul(float4(pos, 1.0f) ,viewproj);
    return output;
}