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
Texture2D Depthtexture1 : register(t9);
Texture2D Depthtexture2 : register(t10);
Texture2D Depthtexture3 : register(t11);
Texture2D DepthBuffer : register(t12);
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
    float4 cascadePlaneDistances;
    float4x4 LightSpaceMatrices0;
    float4x4 LightSpaceMatrices1;
    float4x4 LightSpaceMatrices2;
    float4x4 LightSpaceMatrices3;
    
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
float ShadowCalculation(float3 fragPosWorld, float3 normal, float4x4 view)
{
    float shadow = 0.0f;

    // Transform to view space to determine cascade
    float viewZ = -mul(float4(fragPosWorld, 1.0f), View).z;

    int cascadeIndex = NUM_CASCADES - 1;
    for (int i = 0; i < NUM_CASCADES; ++i)
    {
        if (viewZ < cascadePlaneDistances[i])
        {
            cascadeIndex = i;
            break;
        }
    }
    float4x4 LightSpaceMatrices[4];
    LightSpaceMatrices[0] = LightSpaceMatrices0;
    LightSpaceMatrices[1] = LightSpaceMatrices1;
    LightSpaceMatrices[2] = LightSpaceMatrices2;
    LightSpaceMatrices[3] = LightSpaceMatrices3;
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
float LinearizeDepth(float d)
{
    // d ? [0,1], nearPlane & farPlane in view-space units
    // formula: viewZ = near*far / (far - d*(far - near))
    return (0.1 * 50) /
           (50 - d * (50 - 0.1));
}

// 2) Reconstruct full view-space (or world-space) position
float3 ReconstructViewPos(float2 uv)
{
    // sample the stored depth
    float dNonLin = DepthBuffer.Sample(objSamplerState, uv).r;

    // linear view-space Z
    float viewZ = LinearizeDepth(dNonLin);

    // reconstruct clip-space XY ? [–1,1]
    float2 ndcXY = uv * 2.0f - 1.0f;

    // build a clip-space position (w = 1 after proj)
    float4 clipPos = float4(ndcXY, dNonLin, 1.0f);

    // unproject into view space
    float4 viewH = mul(clipPos, InvView);
    viewH /= viewH.w;

    // view-space position is viewH.xyz
    return viewH.xyz;
}

float4 main( FSInput screenPos): SV_Target0  
{  
    float3 N, P, albedo, spec;
    float roughness, metalness, ao;

    GetGBufferAttributes(screenPos.OutTexCoord, N, P, albedo, spec, metalness);
    P = mul(float4(ReconstructViewPos(screenPos.OutTexCoord), 1.0f), InvView);
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
       
        Lo *= (ShadowCalculation(P, N, View));
        
    }
    float cascadearray[4];
    cascadearray[0] = cascadePlaneDistances.x;
    cascadearray[1] = cascadePlaneDistances.y;
    cascadearray[2] = cascadePlaneDistances.z;
    cascadearray[3] = cascadePlaneDistances.w;
    
    float4 CascadeIndicator = float4(0.0, 0.0, 0.0, 0.0);
    
    float3 viewPos = mul(float4(ReconstructViewPos(screenPos.OutTexCoord),1.0f), InvView);
    
    float4x4 LSM[4];
    LSM[0] = LightSpaceMatrices0;
    LSM[1] = LightSpaceMatrices1;
    LSM[2] = LightSpaceMatrices2;
    LSM[3] = LightSpaceMatrices3;
    for (int j = 0; j < NUM_CASCADES; j++)
    {
        if (viewPos.z <= cascadearray[j])
        {
            
            if (j == 0) 
                CascadeIndicator = float4(0.1, 0.0, 0.0, 0.0);
            else if (j == 1)
                CascadeIndicator = float4(0.0, 0.1, 0.0, 0.0);
            else if (j == 2)
                CascadeIndicator = float4(0.0, 0.0, 0.1, 0.0);
            else if (j == 3)
                CascadeIndicator = float4(0.1, 0.1, 0.0, 0.0);

            break;
        }
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
    
   // return float4(viewPos + CascadeIndicator.xyz, 1.0);
    return float4(color, 1.0);
}
