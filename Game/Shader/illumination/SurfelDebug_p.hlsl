struct PSInput
{
    float4 position : SV_POSITION;
    float2 texcoords : TEXCOORDS;
    float3 color : COLOR;
};


float4 main(PSInput input) : SV_TARGET
{
    float dist = dot(input.texcoords, input.texcoords);
    if (dist > 1.0f)
        discard; // outside the disc
    float3 litColor = input.color;
    return float4(litColor, 1.0);
}