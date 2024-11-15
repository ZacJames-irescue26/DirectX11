struct PSInput  
{  
	float4 PositionSS: SV_Position;  
	float2 TexCoord: TEXCOORD;  
	float3 NormalWS: NORMAL;  
	float3 PositionWS: WORLD_POSITION; 
 };  

struct PSOutput 
{  
	float4 Normal: SV_Target0;  
	float4 DiffuseAlbedo: SV_Target1; 
	float4 SpecularAlbedo: SV_Target2;  
	float4 Position: SV_Target3;  

}; 

Texture2D DiffuseTexture : TEXTURE: register(t0);
//Texture2D NormalTexture : TEXTURE: register(t1);
//Texture2D MetallicTexture : TEXTURE : register(t2);
//Texture2D RoughnessTexture : TEXTURE : register(t3);


SamplerState objSamplerState : SAMPLER: register(s0);



PSOutput main( PSInput input)
{
	PSOutput output;  
	// Sample the diffuse map  
	float3 diffuseAlbedo = DiffuseTexture.Sample( objSamplerState, input.TexCoord ).rgb;  
	// Normalize the normal after interpolation  
	float3 normalWS = normalize( input.NormalWS);  
	// Output our G-Buffer values  
	/* output.Normal = float4( normalWS, 1.0f); 
	output.DiffuseAlbedo = float4( diffuseAlbedo, 1.0f);  
	output.SpecularAlbedo = float4( 1.0,1.0,1.0, 1.0);
	output.Position = float4( input.PositionWS, 1.0f);  */

	output.Normal = float4( 1.0,0.0,0.0, 1.0f); 
	output.DiffuseAlbedo = float4( 1.0,0.0,0.0, 1.0f);  
	output.SpecularAlbedo = float4( 1.0,1.0,1.0, 1.0);
	output.Position = float4( 1.0,0.0,0.0, 1.0f);  

	return output;

}