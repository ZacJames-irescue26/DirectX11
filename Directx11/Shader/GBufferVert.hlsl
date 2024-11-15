cbuffer buffer : register(b0)
{
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
};


// Input/Output structures  
struct VSInput  {  
	float3 Position: POSITION;  
	float2 TexCoord: TEXCOORD;  
	float3 Normal: NORMAL;  
};  
struct VSOutput  {  
	float4 PositionCS: SV_Position;  
	float2 TexCoord: TEXCOORD;  
	float3 NormalWS: NORMAL;  
	float3 World_Position: WORLD_POSITION;  
}; 






VSOutput main( VSInput input )
{

	VSOutput output;

	// Convert position and normals to world space  
	output.World_Position = mul( float4(input.Position,1.0), worldMatrix ).xyz;  
	output.NormalWS = normalize( mul( input.Normal, (float3x3)worldMatrix));  
	// Calculate the clip-space position  
	output.PositionCS = mul( input.Position, wvpMatrix);  
	// Pass along the texture coordinate  
	output.TexCoord = input.TexCoord;
	return output;

}