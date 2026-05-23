
struct PSInput
{
    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;
};


Texture2D inTex : register(t0);
SamplerState inSampler : register(s0);


float4 main(PSInput input) : SV_TARGET0
{
    return inTex.Sample(inSampler, input.inTexCoord);
}