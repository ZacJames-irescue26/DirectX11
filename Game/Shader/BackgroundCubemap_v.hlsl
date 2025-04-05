cbuffer ViewProj : register(b0)
{
    float4x4 proj;
    float4x4 view;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_Position;
    float3 localPos : TEXCOORD;
};

VS_OUTPUT main(float3 pos : POSITION)
{
    VS_OUTPUT output;

    float4x4 rotView = float4x4(
    view[0].xyz, 0.0,
    view[1].xyz, 0.0,
    view[2].xyz, 0.0,
    0.0, 0.0, 0.0, 1.0
);

    float4x4 viewProj = mul(rotView, proj);
    output.outPosition = mul(float4(pos, 1.0), viewProj);

    // We use the vertex position as direction for cubemap sampling
    output.localPos = pos;

    return output;
}