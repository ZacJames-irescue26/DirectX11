struct VSInput
{
	float3 inPos : POSITION;
	float2 inTexCoord : TEXCOORD;
};

struct VSOutput
{

	float4 outPosition : SV_POSITION;
	float2 OutTexCoord : TEXCOORD;

};



VSOutput main( VSInput input )
{
	VSOutput output;

	output.outPosition = float4(input.inPos, 1.0f);
	output.OutTexCoord = input.inTexCoord;

	return output;
}