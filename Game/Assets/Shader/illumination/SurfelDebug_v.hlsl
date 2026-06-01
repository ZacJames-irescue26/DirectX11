struct VSInput
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 color : COLOR;
    float3 indirectRadiance : Radiance;
    float radius : RADIUS;
};

struct GSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float radius : RADIUS;
};

cbuffer Camera : register(b0)
{
    float4x4 proj;
    float4x4 view;
};

GSInput main(VSInput input)
{
    GSInput output;
    
    float4 worldPos = float4(input.position.xyz, 1.0f);
    float4 viewPos = mul(worldPos, view);
    
    output.position = mul(viewPos, proj); // For rasterization
    output.worldPos = input.position.xyz; // World position
    output.normal = normalize(input.normal.xyz);
    output.color = input.color;
    output.radius = input.radius;

    return output;
}