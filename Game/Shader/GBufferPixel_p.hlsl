struct PSInput  
{  
	float4 PositionSS: SV_Position;  
	float2 TexCoord: TEXCOORD;  
	float3 NormalWS: NORMAL;  
	float3 PositionWS: WORLD_POSITION; 
    float3 Tangent : TANGENT;
    float3 BiTangent : BITANGENT;
};  

struct PSOutput 
{  
	float4 Normal: SV_Target0;  
	float4 DiffuseAlbedo: SV_Target1; 
    float4 Roughness : SV_Target2; // roughness, metal, ao, etc.
	float4 Position: SV_Target3;  

}; 

Texture2D DiffuseTexture : TEXTURE: register(t0);
Texture2D NormalTexture : TEXTURE: register(t1);
Texture2D RoughnessTexture : TEXTURE : register(t2);



SamplerState objSamplerState : SAMPLER: register(s0);


float3 getNormalFromMap(float2 TexCoords, float3 WorldPos, float3 Normal)
{
    float3 tangentNormal = NormalTexture.Sample(objSamplerState, TexCoords).rgb;
	// Optional: Flip green channel if needed (for OpenGL-style normal maps)
    tangentNormal.g = 1.0f - tangentNormal.g;

    tangentNormal = tangentNormal * 2.0 - 1.0;
    float3 Q1 = ddx(WorldPos);
    float3 Q2 = ddy(WorldPos);
    float2 st1 = ddx(TexCoords);
    float2 st2 = ddy(TexCoords);

    float3 N = normalize(Normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(normalize(T), normalize(B), normalize(N));

    return normalize(mul(TBN, tangentNormal));
}
PSOutput main( PSInput input)
{
	PSOutput output;  
	
	
	// Sample the diffuse map  
	float3 diffuseAlbedo = DiffuseTexture.Sample( objSamplerState, input.TexCoord ).rgb;  
	// Normalize the normal after interpolation  

    // 3. Convert to world-space normal
    float3 worldNormal = getNormalFromMap(input.TexCoord, input.PositionWS, input.NormalWS);
    float3 normalTS = NormalTexture.Sample(objSamplerState, input.TexCoord).rgb;
    normalTS = normalTS * 2.0f - 1.0f; // Tangent space normal: [-1, 1]
    float3 T = normalize(input.Tangent);
    float3 B = normalize(input.BiTangent);
    float3 N = normalize(input.NormalWS);
    float3x3 TBN = float3x3(T, B, N);
    float3 normalWS = normalize(mul(normalTS, TBN));
	// === 3. Sample Roughness (metalness optional) ===
    float3 roughness = RoughnessTexture.Sample(objSamplerState, input.TexCoord).rgb; // Assuming grayscale roughness
    float metalness = 0.0f; // Set manually or use a MetalnessMap (register t3)
	// Output our G-Buffer values  
    output.Normal = float4(normalWS, 1.0f);
	output.DiffuseAlbedo = float4( diffuseAlbedo, 1.0f);  
    output.Roughness = float4(roughness,1.0);
	output.Position = float4( input.PositionWS, 1.0f);

	//output.Normal = float4( 0.0,1.0,0.0, 1.0f); 
	//output.DiffuseAlbedo = float4( 1.0,1.0,1.0, 1.0f);  
	//output.SpecularAlbedo = float4( 1.0,1.0,1.0, 1.0);
	//output.Position = float4( 1.0,1.0,0.0, 1.0f);  

	return output;

}