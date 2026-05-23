// Input struct from vertex shader
struct VS_OUTPUT
{
    float4 position : SV_Position;
};

// Geometry shader input: 3 vertices for a triangle
struct GS_INPUT
{
    float4 position : SV_Position;
};

// Geometry shader output
struct GS_OUTPUT
{
    float4 position : SV_Position;
    uint layer : SV_RenderTargetArrayIndex; // for rendering to texture array layers
};

// Constant buffer with light-space matrices
cbuffer LightSpaceMatrices : register(b0)
{
    float4x4 lightSpaceMatrices[5]; // Adjust size as needed
};

// Geometry Shader
[maxvertexcount(3)]
void main(triangle GS_INPUT input[3], in uint invocationID : SV_GSInstanceID, inout TriangleStream<GS_OUTPUT> triStream)
{
    GS_OUTPUT output;

    for (int i = 0; i < 3; ++i)
    {
        output.position = mul(input[i].position, lightSpaceMatrices[invocationID]);
        output.layer = invocationID; // Write to the correct layer
        triStream.Append(output);
    }

    triStream.RestartStrip();
}