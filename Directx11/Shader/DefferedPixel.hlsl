// Textures  
Texture2D NormalTexture: register( t0);  
Texture2D DiffuseAlbedoTexture: register( t1);  
Texture2D SpecularAlbedoTexture: register( t2);  
Texture2D PositionTexture: register( t3);  

// Constants  
cbuffer LightParams  
{  
    float3 LightPos;  
    float3 LightColor; 
     float3 LightDirection;  
    float2 SpotlightAngles;  
    float4 LightRange;  
}; 

 cbuffer CameraParams  
{  
    float3 CameraPos;  
};  


// Helper function for extracting G-Buffer attributes  
void GetGBufferAttributes( in float2 screenPos, out float3 normal,  out float3 position,  
out float3 diffuseAlbedo, out float3 specularAlbedo,  
out float specularPower)  
{ 

     // Determine our indices for sampling the texture based on the current  
    // screen position  
    int3 sampleIndices = int3( screenPos.xy, 0); 
     normal = NormalTexture.Load( sampleIndices ).xyz;  
    position = PositionTexture.Load( sampleIndices ).xyz;  
    diffuseAlbedo = DiffuseAlbedoTexture.Load( sampleIndices ).xyz;  
    float4 spec = SpecularAlbedoTexture.Load( sampleIndices);  
    specularAlbedo = spec.xyz;  
    specularPower = 1.0;  
}  

// Calculates the lighting term for a single G-Buffer texel  

float3 CalcLighting( in float3 normal,  in float3 position,  
    in float3 diffuseAlbedo,  in float3 specularAlbedo,  in float specularPower)  
{  
    // Calculate the diffuse term  
    float3 L = 0;  float attenuation = 1.0f;  
//#if POINTLIGHT || SPOTLIGHT  
//    // Base the the light vector on the light position 
//    L = LightPos - position;
//
//    // Calculate attenuation based on distance from the light source  
//    float dist = length( L); 
//     attenuation = max( 0, 1.0f - (dist/ LightRange.x)); 
//     L /= dist;  
//#elif DIRECTIONALLIGHT  
    // Light direction is explicit for directional lights  
    L = -LightDirection;  
//#endif  
//#if SPOTLIGHT  
//    // Also add in the spotlight attenuation factor  
//    float3 L2 = LightDirection;  
//    float rho = dot( -L, L2);  attenuation *= saturate( (rho - SpotlightAngles.y)  / (SpotlightAngles.x -  SpotlightAngles.y));  
//#endif  
    float nDotL = saturate( dot( normal, L));  
    float3 diffuse = nDotL * LightColor * diffuseAlbedo;  
    // Calculate the specular term  
    float3 V = CameraPos - position;  
    float3 H = normalize( L + V);  
    float3 specular = pow( saturate( dot( normal, H)), specularPower)  * LightColor * specularAlbedo.xyz * nDotL;  
    // Final value is the sum of the albedo and diffuse with attenuation applied 
     return (diffuse + specular) * attenuation; 
}  
// Lighting pixel shader 

struct FSInput
{

	float4 outPosition : SV_POSITION;
	float2 OutTexCoord : TEXCOORD;

};
 
float4 main( FSInput screenPos): SV_Target0  
{  
    float3 normal;  float3 position; 
    float3 diffuseAlbedo;  float3 specularAlbedo;  float specularPower; 
     // Sample the G-Buffer properties from the textures  
    GetGBufferAttributes( screenPos.outPosition.xy, normal, position, diffuseAlbedo,  specularAlbedo, specularPower);  
    //float3 lighting = CalcLighting( normal, position, diffuseAlbedo,  specularAlbedo, specularPower);  
   
    float3 lighting = (1.0,1.0,1.0);
    return float4( lighting, 1.0f);  
}
