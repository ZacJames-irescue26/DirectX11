

struct PSInput
{

    float4 inPosition : SV_POSITION;
    float2 inTexCoord : TEXCOORD;

};

Texture2D shadowmap : TEXTURE : register(t0);
SamplerState objsampler : SAMPLER : register(s0);

float LinearizeDepth(float depth, float nearZ, float farZ)
{
    return (2.0 * nearZ) / (farZ + nearZ - depth * (farZ - nearZ));
}
float4 main(PSInput input) : SV_TARGET
{
    float3 shadow = shadowmap.Sample(objsampler, input.inTexCoord).rrr;
   
	return float4(shadow.rrr, 1.0f);
}