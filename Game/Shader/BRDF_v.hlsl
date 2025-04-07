struct VS_INPUT
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 outPos : SV_Position;
    float2 outTexcoords : TexCoords;
};

VS_OUTPUT main( VS_INPUT input )
{
    VS_OUTPUT output;
    output.outPos = float4(input.Position, 0.0, 1.0f);
    output.outTexcoords = input.TexCoord;
    
	return output;
}