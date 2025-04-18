

cbuffer ViewProj
{
    float4x4 proj;
    float4x4 view;
};


float4 main( float3 pos : POSITION ) : SV_POSITION
{
    float4 worldPos = float4(pos, 1.0f);
    return mul(worldPos, mul(view, proj));
}