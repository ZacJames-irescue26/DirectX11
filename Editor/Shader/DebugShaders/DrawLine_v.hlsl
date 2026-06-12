cbuffer DebugCB : register(b0)
{
    float4x4 ViewProjection;
};

struct VSIn
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct VSOut
{
    float4 Position : SV_POSITION;
    float3 Color : COLOR;
};

VSOut main(VSIn input)
{
    VSOut output;
    output.Position = mul(float4(input.Position, 1.0f), ViewProjection);
    output.Color = input.Color;
    return output;
}