// Textures  
Texture2D NormalTexture: register( t0);  
Texture2D DiffuseAlbedoTexture: register( t1);  
Texture2D SpecularAlbedoTexture: register( t2);  
Texture2D PositionTexture: register( t3); 
TextureCube irradianceMap : register(t4);
TextureCube SkyBoxMap : register(t5);
TextureCube prefilterMap : register(t6);
Texture2D brdfLUT : register(t7);
Texture2D Depthtexture0 : register(t8);

SamplerState objSamplerState : SAMPLER : register(s0);
SamplerState irradianceSamplerstate : SAMPLER : register(s1);
SamplerComparisonState ShadowSampler : register(s2);

#define PI 3.141595
// Constants 

#define NUM_CASCADES 4

cbuffer LightParams : register(b0)
{   
    float3 LightColor; 
    float padding;
    float3 LightDirection;  
    float farPlane;
    float4x4 LightSpaceMatrices;
    
}; 



cbuffer CameraParams : register(b1) 
{  
    float4x4 InvProj;
    float4x4 InvView;
    float4x4 View;
    float3 CameraPos; 
    float padding2;
};  

struct CastLight
{
    float3 intensity;
    float padding;
    float3 position;
    float padding1;
    float3 direction;
    float cutOff;
};
cbuffer Lights :register(b2)
{
    CastLight light;
};
/*float ShadowCalculation(float3 fragPosWorld, float3 normal, float4x4 view)
{
    float shadow = 0.0f;

    // Transform to view space to determine cascade
    float4 fragView = mul(float4(fragPosWorld, 1.0f), view);
    float depthValue = abs(fragView.z);

    int cascadeIndex = NUM_CASCADES - 1;
    for (int i = 0; i < NUM_CASCADES; ++i)
    {
        if (depthValue < cascadePlaneDistances[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    float4 fragLightSpace = mul(float4(fragPosWorld, 1.0f), LightSpaceMatrices[cascadeIndex]);
    float3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;

    if (projCoords.z > 1.0f)
        return 0.0f;

    float3 N = normalize(normal);
    float3 L = normalize(-LightDirection);
    float bias = max(0.05f * (1.0f - dot(N, L)), 0.005f);

    float biasModifier = 0.5f;
    float cascadeDepth = (cascadeIndex == NUM_CASCADES - 1) ? farPlane : cascadePlaneDistances[cascadeIndex];
    bias *= 1.0f / (cascadeDepth * biasModifier);

    float2 texelSize;
    uint width, height;
    Depthtexture1.GetDimensions(width, height);
    texelSize = 1.0f / float2(width, height);
    float shadowSum = 0.0f;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            float2 sampleUV = projCoords.xy + offset;

            float comparison = 0.0f;
            switch (cascadeIndex)
            {
                case 0:
                    comparison = Depthtexture0.SampleCmpLevelZero(ShadowSampler, sampleUV, projCoords.z - bias);
                    break;
                case 1:
                    comparison = Depthtexture1.SampleCmpLevelZero(ShadowSampler, sampleUV, projCoords.z - bias);
                    break;
                case 2:
                    comparison = Depthtexture2.SampleCmpLevelZero(ShadowSampler, sampleUV, projCoords.z - bias);
                    break;
                case 3:
                    comparison = Depthtexture3.SampleCmpLevelZero(ShadowSampler, sampleUV, projCoords.z - bias);
                    break;
            }

            shadowSum += comparison;
        }
    }

    shadow = shadowSum / 9.0f;
    return 1.0f - shadow; // Shadow amount (1 = fully shadowed, 0 = lit)
}*/

float ShadowCalculation(float3 fragPosWorld, float3 normal, float3 lightDir)
{
    // Transform to light space
    float4 fragLightSpace = mul(float4(fragPosWorld, 1.0f), LightSpaceMatrices);

    // Perspective divide
    float3 projCoords = fragLightSpace.xyz / fragLightSpace.w;

    // Transform to [0, 1] range
    projCoords = projCoords * 0.5f + 0.5f;

    // If outside the shadow map
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || projCoords.z > 1.0)
        return 0.0f;

    // Calculate bias
    float bias = max(0.05f * (1.0f - dot(normalize(normal), normalize(lightDir))), 0.1f);
    bias = 0.005f;
    // PCF
    float2 texelSize = 1.0f / float2(2048.0f, 2048.0f); // Replace with your actual resolution
    float shadow = 0.0f;

    [unroll]
    for (int x = -1; x <= 1; ++x)
    {
        [unroll]
        for (int y = -1; y <= 1; ++y)
        {
            float2 offset = float2(x, y) * texelSize;
            shadow += Depthtexture0.SampleCmpLevelZero(ShadowSampler, projCoords.xy + offset, projCoords.z - bias);
        }
    }

    shadow /= 9.0f; 

    return 1.0f - shadow; // 1 = shadowed, 0 = lit
}

float3 Eval_CastLight(const CastLight light, const float3 Pos, const float3 N)
{
    // check if lighting is inside the spotlight cone
    float3 lightDir = normalize(light.position - Pos);
    float theta = dot(lightDir, normalize(-light.direction));
    if (theta > light.cutOff)
    {
        float incoming_cos = max(dot(lightDir, N), 0);
        float dist = length(light.position - Pos);
        float softedge = (theta - light.cutOff) / (1 - light.cutOff);
        float3 radiance = softedge * light.intensity * incoming_cos / (dist * dist);
        return radiance;
    }
    return float3(0, 0, 0);
}

float3 Eval_ParalLight(float3 lightdir, const float3 N)
{
    float3 L = normalize(lightdir); // Light direction *to* the surface
    float NdotL = max(dot(N, L), 0.0);
    return LightColor * NdotL;
}

float ShadowCalculation1(float3 fragPos, float3 Normal, float3 lightdir)
{
    float4 fragPosLightSpace = mul(LightSpaceMatrices, float4(fragPos, 1.0f));
    // perform perspective divide
   float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0)
        return 0.0;
    
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = Depthtexture0.Sample(objSamplerState, projCoords.xy).r;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    float3 normal = normalize(Normal);
    float3 lightDir = normalize(lightdir);
    float bias = 0.1 * max(0.05 * (1.0 - dot(normal, lightDir)), 0.01);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    uint width, height;
    Depthtexture0.GetDimensions(width, height);
    shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    //float2 texelSize = 1.0 / float2(width, height);
    //for (int x = -1; x <= 1; ++x)
   // {
    //    for (int y = -1; y <= 1; ++y)
    //    {
    //        float pcfDepth = Depthtexture0.Sample(objSamplerState, projCoords.xy + float2(x, y) * texelSize).r;
    //        shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
    //    }
    //}
   // shadow /= 9.0;

    return shadow;
}


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
    float roughness, metalness, ao;

    GetGBufferAttributes(screenPos.OutTexCoord, N, P, albedo, spec, metalness);
    
    if (length(P.xyz) == 0.0f)
    {
    // No geometry, output skybox sample
        return SkyBoxMap.Sample(
        irradianceSamplerstate, ReconstructViewDir(screenPos.OutTexCoord));
    }
    ao = spec.r;
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
    
    for (int i = 0; i < 1; i++)
    {
        float3 WorldPos = P;
		// calculate per-light radiance
        float3 L = normalize(-LightDirection); // Use direction as-is
        float3 H = normalize(L + V);
        float3 radiance = Eval_ParalLight(-LightDirection, N); // No attenuation for directional light

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
        Lo *= (1.0f-ShadowCalculation1(P, N, LightDirection));
        
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


    float2 brdf = brdfLUT.Sample(objSamplerState, float2(dot(N, V),  roughness)).rg;
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    float ambientscalar = 0.02;
    float3 ambient = (kD * diffuse + specular) /** ao*/* ambientscalar;
    float3 color = Lo+ambient;
	// HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
	// gamma correct
    color = pow(color, float3((1.0 / 2.2), (1.0 / 2.2), (1.0 / 2.2)));
    float4 fragPosLightSpace = mul(LightSpaceMatrices, float4(P, 1.0f));
    if (fragPosLightSpace.w == 0.0f)
        return float4(1, 0, 0, 1); // Debug fallback
    // perform perspective divide
    float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    projCoords = clamp(projCoords * 0.5f + 0.5f, 0.0f, 1.0f);
    float current = projCoords.z;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = Depthtexture0.Sample(objSamplerState, projCoords.xy).r;
    return float4(color, 1.0);
}
