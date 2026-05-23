
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
    VS_OUTPUT OUT;
    // Build projection×view in correct order
    float4x4 vp = mul(proj, view);

    // Transform the cube vertex into clip space
    OUT.outPosition = mul(float4(pos, 1), vp);

    // Pass the *direction* for texturing
    OUT.localPos = pos;
    return OUT;
}