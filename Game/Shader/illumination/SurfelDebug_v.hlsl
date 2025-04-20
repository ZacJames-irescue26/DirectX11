struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float4 color : COLOR;
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
    
    float4 worldPos = float4(input.position, 1.0f);
    float4 viewPos = mul(worldPos, view);
    
    output.position = mul(viewPos, proj); // For rasterization
    output.worldPos = input.position; // World position
    output.normal = normalize(input.normal);
    output.color = input.color;
    output.radius = input.radius;

    return output;
}