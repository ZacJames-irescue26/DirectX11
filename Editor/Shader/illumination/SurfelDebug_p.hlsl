struct PSInput
{
    float4 position : SV_POSITION;
    float3 worldPos : WORLDPOS;
    float3 normal : NORMAL;
    float4 color : COLOR;
    float radius : RADIUS;
};


float4 main(PSInput input) : SV_TARGET
{
    //float dist = dot(input.texcoords, input.texcoords);
    //if (dist > 0.01f)
    //    discard; // outside the disc
    float3 litColor = input.color;
    return float4(litColor, 1.0);
}