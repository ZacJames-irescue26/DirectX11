// Textures  
Texture2D NormalTexture: register( t0);  
Texture2D DiffuseAlbedoTexture: register( t1);  
Texture2D SpecularAlbedoTexture: register( t2);  
Texture2D PositionTexture: register( t3); 
TextureCube irradianceMap : register(t4);
TextureCube SkyBoxMap : register(t5);
TextureCube prefilterMap : register(t6);
Texture2D brdfLUT : register(t7);

SamplerState objSamplerState : SAMPLER : register(s0);
SamplerState irradianceSamplerstate : SAMPLER : register(s1);
#define PI 3.141595
// Constants  
cbuffer LightParams : register(b0)
{   
    float3 LightColor; 
    float padding;
    float3 LightDirection;  
    float padding1;
}; 

cbuffer CameraParams : register(b1) 
{  
    float4x4 InvProj;
    float4x4 InvView;
    float3 CameraPos; 
    float padding2;
};  


// Helper function for extracting G-Buffer attributes  
void GetGBufferAttributes( in float2 screenPos, out float3 normal,  out float3 position,  
out float3 diffuseAlbedo, out float3 specularAlbedo,  
out float specularPower)  
{ 

     // Determine our indices for sampling the texture based on the current  
    // screen position  
    int3 sampleIndices = int3( screenPos.xy, 0); 
     normal = NormalTexture.Sample(objSamplerState, screenPos).xyz;  
    position = PositionTexture.Sample(objSamplerState, screenPos).xyz;
    diffuseAlbedo = DiffuseAlbedoTexture.Sample(objSamplerState, screenPos).xyz;
    float4 spec = SpecularAlbedoTexture.Sample(objSamplerState, screenPos);
    specularAlbedo = spec.xyz;  
    specularPower = 1.0;  
}  

// Calculates the lighting term for a single G-Buffer texel  
// ----------------------------------------------------------------------------
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
	
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// ----------------------------------------------------------------------------
float3 fresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}
// Lighting pixel shader 

struct FSInput
{

	float4 outPosition : SV_POSITION;
	float2 OutTexCoord : TEXCOORD;

};
float3 ReconstructViewDir(float2 UV)
{
    float2 ndc = UV * 2.0f - 1.0f; // from [0,1] to [-1,1]
    // Reconstruct clip space position
    float4 clipPos = float4(ndc.x, -ndc.y, 1.0f, 1.0f); // Z = 1 for far plane

// Reconstruct view space position
    float4 viewDir4 = mul(clipPos, InvProj);
    float3 viewDir = normalize(viewDir4.xyz / viewDir4.w);
    float4 worldDir4 = mul(float4(viewDir, 0.0f), InvView);
    float3 worldDir = normalize(worldDir4.xyz);
    return worldDir;
}

float4 main( FSInput screenPos): SV_Target0  
{  
    float3 N, P, albedo, spec;
    float roughness, metalness;

    GetGBufferAttributes(screenPos.OutTexCoord, N, P, albedo, spec, metalness);
    
    if (length(P.xyz) == 0.0f)
    {
    // No geometry, output skybox sample
        return SkyBoxMap.Sample(
        irradianceSamplerstate, ReconstructViewDir(screenPos.OutTexCoord));
    }
    
    roughness = spec.g;
    metalness = 0.0f;
    float3 V = normalize(CameraPos - P);
    float3 R = reflect(-V, N);
   	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metalness);
    	// reflectance equation
    float3 Lo = float3(0.0, 0.0, 0.0);
    
    for (int i = 0; i < 2; i++)
    {
        float3 WorldPos = P;
		// calculate per-light radiance
        float3 L = normalize(-LightDirection); // Use direction as-is
        float3 H = normalize(L + V);
        float3 radiance = LightColor; // No attenuation for directional light

		// Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        float3 numerator = NDF * G * F;
        float denominator = mul(mul(4, max(dot(N, V), 0.0)), max(dot(N, L), 0.0)) + 0.0001; // + 0.0001 to prevent divide by zero
        float3 specular = numerator / denominator;

		// kS is equal to Fresnel
        float3 kS = F;
		// for energy conservation, the diffuse and specular light can't
		// be above 1.0 (unless the surface emits light); to preserve this
		// relationship the diffuse component (kD) should equal 1.0 - kS.
        float3 kD = float3(1.0, 1.0, 1.0) - kS;
		// multiply kD by the inverse metalness such that only non-metals 
		// have diffuse lighting, or a linear blend if partly metal (pure metals
		// have no diffuse light).
        kD *= 1.0 - metalness;

		// scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

		// add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }
    float3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	// ambient lighting (note that the next IBL tutorial will replace 
	// this ambient lighting with environment lighting).
    // ambient lighting (we now use IBL as the ambient term)
    float3 kS = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metalness;
    float3 irradiance = irradianceMap.Sample(irradianceSamplerstate, N).rgb;
    float3 diffuse = irradiance * albedo;

        // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    float3 prefilteredColor = prefilterMap.SampleLevel(irradianceSamplerstate, R, roughness * MAX_REFLECTION_LOD).rgb;


    float2 brdf = brdfLUT.Sample(objSamplerState, float2(max(dot(N, V), 0.0), roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    float3 ambient = (kD * diffuse + specular) /* * ao*/;
    float3 color = Lo+ambient;
	// HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
	// gamma correct
    color = pow(color, float3((1.0 / 2.2), (1.0 / 2.2), (1.0 / 2.2)));

    return float4(color, 1.0);
}
