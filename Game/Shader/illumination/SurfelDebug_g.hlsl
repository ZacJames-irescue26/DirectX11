
struct GSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float3 color : COLOR;
    float radius : RADIUS;
};
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoords : TEXCOORDS;
    float3 color : COLOR;
};
cbuffer Camera : register(b0)
{
    float4x4 proj;
    float4x4 view;
};
[maxvertexcount(4)]
void main(point GSInput input[1], inout TriangleStream<PSInput> triStream)
{
    float3 center = input[0].worldPos;
    float3 normal = normalize(input[0].normal);
    float3 color = input[0].color.rgb;
    float radius = input[0].radius;

    // Generate a local tangent/bitangent basis
    float3 tangent = normalize(cross(float3(0.0f, 1.0f, 0.0f), normal));
    if (length(tangent) < 0.01f)
    {
        tangent = normalize(cross(float3(1.0f, 0.0f, 0.0f), normal));
    }
    float3 bitangent = normalize(cross(normal, tangent));

    // Define quad corners (you could do more for better circular shape)
    float3 offset0 = (-tangent - bitangent) * radius;
    float3 offset1 = (-tangent + bitangent) * radius;
    float3 offset2 = (tangent + bitangent) * radius;
    float3 offset3 = (tangent - bitangent) * radius;

    float4 viewPos;
    PSInput outVert;

    float2 uvs[4] =
    {
        float2(-1, -1),
        float2(-1, 1),
        float2(1, 1),
        float2(1, -1)
    };
    
    // Emit triangle strip (quad)
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        float3 offset = (i == 0) ? offset0 :
                        (i == 1) ? offset1 :
                        (i == 2) ? offset2 :
                                   offset3;

        float3 worldPos = center + offset;
        viewPos = mul(float4(worldPos, 1.0f), view);
        outVert.position = mul(viewPos, proj);
        outVert.color = color;
        outVert.texcoords = uvs[i]; // Set here!
        triStream.Append(outVert);
    }
    triStream.RestartStrip();
}