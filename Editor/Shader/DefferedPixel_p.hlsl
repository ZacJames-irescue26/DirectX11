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
Texture2D DirectionalSM : register(t13);
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
    row_major float4x4 LSMDirectionalShadow;
    
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

float3 ReconstructWorldPos(float2 uv)
{
    float d = DepthBuffer.Sample(objSamplerState, uv).r;
    
    float2 ndcXY = uv * 2.0f - 1.0f;
    float ndcZ = d * 2.0f - 1.0f;

    float4 clipPos = float4(ndcXY, ndcZ, 1.0f);
    float4 world = mul(clipPos, mul(InvProj, InvView));
    return world.xyz / world.w;
}

float ShadowCalculation(float3 fragPosWorld, float3 normal)
{
    float3 fragPosView = mul(float4(fragPosWorld, 1.0f), View).xyz;
    float viewZ = -fragPosView.z;

    int cascadeIndex = NUM_CASCADES - 1;
    for (int i = 0; i < NUM_CASCADES; ++i)
    {
        if (viewZ < cascadePlaneDistances[i])
        {
            cascadeIndex = i;
            break;
        }
    }

    float4x4 LightSpaceMatrices[NUM_CASCADES] =
    {
        LightSpaceMatrices0, LightSpaceMatrices1,
        LightSpaceMatrices2, LightSpaceMatrices3
    };

    float4 fragLightSpace = mul(float4(fragPosWorld, 1.0f), LightSpaceMatrices[cascadeIndex]);
    float3 projCoords = fragLightSpace.xyz / fragLightSpace.w;
    projCoords = projCoords * 0.5f + 0.5f;

    if (projCoords.z > 1.0f)
        return 0.0f;

    float3 N = normalize(normal);
    float3 L = normalize(-LightDirection);
    float bias = max(0.005f * (1.0f - dot(N, L)), 0.0005f);

    uint width, height;
    Depthtexture0.GetDimensions(width, height);
    float2 texelSize = 1.0f / float2(width, height);

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
    float comp;
    comp = shadowSum / 9.0f;
    return 1.0f - comp;
}

static const float2 poissonDisk[16] =
{
    float2(-0.94201624, -0.39906216),
   float2(0.94558609, -0.76890725),
   float2(-0.094184101, -0.92938870),
   float2(0.34495938, 0.29387760),
   float2(-0.91588581, 0.45771432),
   float2(-0.81544232, -0.87912464),
   float2(-0.38277543, 0.27676845),
   float2(0.97484398, 0.75648379),
   float2(0.44323325, -0.97511554),
   float2(0.53742981, -0.47373420),
   float2(-0.26496911, -0.41893023),
   float2(0.79197514, 0.19090188),
   float2(-0.24188840, 0.99706507),
   float2(-0.81409955, 0.91437590),
   float2(0.19984126, 0.78641367),
   float2(0.14383161, -0.14100790)
};

float DirectionalShadowCalculation_Poisson(
    float3 P_ws, float3 N, float3 lightDir)
{

    float4 lsH = mul(float4(P_ws, 1), LSMDirectionalShadow);
    float3 proj = lsH.xyz / lsH.w * 0.5 + 0.5;
    if (proj.x < 0 || proj.x > 1 || proj.y < 0 || proj.y > 1 || proj.z > 1)
        return 0.0f;

  
    float bias = max(0.005f * (1 - dot(N, lightDir)), 0.0005f);
    float2 texel = 1.0f / 2048; 

   
    float rnd = frac(sin(dot(proj.xy, float2(12.9898, 78.233))) * 43758.5453);
    float angle = rnd * 6.2831853; // 2?
    float2x2 rot = float2x2(cos(angle), -sin(angle),
                             sin(angle), cos(angle));


    const float radius = 3.0; 
    float visibility = 0.0f;
    [unroll]
    for (int i = 0; i < 16; ++i)
    {
        // rotate & scale the disk sample
        float2 offs = mul(rot, poissonDisk[i]) * texel * radius;
        // compare the shadow map at that offset
        visibility += DirectionalSM.SampleCmpLevelZero(
                          ShadowSampler,
                          proj.xy + offs,
                          proj.z - bias);
    }
    visibility /= 16.0f;

    // return fraction in shadow
    return 1.0f - visibility;
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



float4 main1(float3 albedo, float3 normal, float3 position, float3 lightdir)
{
    
    float3 color = albedo;
    float3 norm = normalize(normal);
    float3 lightColor = float3(0.3, 0.3,0.3);
    // ambient
    float3 ambient = 0.3 * lightColor;
    // diffuse
    float diff = max(dot(lightdir, normal), 0.0);
    float3 diffuse = diff * lightColor;
    // specular
    float3 viewDir = normalize(CameraPos - position);
    float3 reflectDir = reflect(-lightdir, normal);
    float spec = 0.0;
    float3 halfwayDir = normalize(lightdir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    float3 specular = spec * lightColor;
    // calculate shadow
    float shadow = ShadowCalculation(position, norm);
    float3 lighting = (ambient + (1.0f - shadow) * (diffuse + specular)) * color;
    
    return float4(lighting, 1.0);
}
float4 main_debugCascade(float3 position)
{
    // 1) compute view-space Z as before
    float viewZ = -mul(float4(position, 1), View).z;

    // 2) normalize to [0,1] for our gradient
    float g = saturate(viewZ / 100);

    // 3) define split distances (in the same units as viewZ!)
    float splits[4] =
    {
        cascadePlaneDistances.x,
        cascadePlaneDistances.y,
        cascadePlaneDistances.z,
        cascadePlaneDistances.w
    };

    // 4) remap splits into [0,1] as well
    float s0 = splits[0] / 100;
    float s1 = splits[1] / 100;
    float s2 = splits[2] / 100;
    float s3 = splits[3] / 100;

    // 5) if g is within ±epsilon of a split, draw a bright line
    const float eps = 0.002; // tweak thickness
    if (abs(g - s0) < eps)
        return float4(1, 0, 0, 0); // red
    if (abs(g - s1) < eps)
        return float4(0, 1, 0, 0); // green
    if (abs(g - s2) < eps)
        return float4(0, 0, 1, 0); // blue
    if (abs(g - s3) < eps)
        return float4(1, 1, 0, 0); // yellow

    // 6) otherwise just show the gradient
    return float4(g, g, g, 1);
}

float4 main_debugViewZ(float3 worldPos)
{
   
    // visualize raw world-space Z
    float v = saturate((worldPos.z + 1) * 0.5);
    // (assumes your scene’s Z ? [–1,+1]; tweak as needed)
    return float4(v, v, v, 1);
}

float4 main( FSInput screenPos): SV_Target0  
{  
    float3 N, P, albedo, spec;
    float roughness, metalness, ao;

    GetGBufferAttributes(screenPos.OutTexCoord, N, P, albedo, spec, metalness);
    P = ReconstructWorldPos(screenPos.OutTexCoord);
    if (length(P.xyz) == 0.0f)
    {
    // No geometry, output skybox sample
        return SkyBoxMap.Sample(
        irradianceSamplerstate, ReconstructViewDir(screenPos.OutTexCoord));
    }
    //ao = spec.r;
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
    float comp;
    [unroll]
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
       
       // Lo *= (1.0f-DirectionalShadowCalculation(P, N, LightDirection));
        
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
    float shadow = DirectionalShadowCalculation_Poisson(P, N, LightDirection);
    float3 direct = (diffuse + specular) * (1 - shadow);

// --- IBL ambient (no shadows) ---
    float3 ambient = kD * diffuse // from irradiance map
               + specular; // from prefiltered map + BRDF LUT
    //ambient *= ao; // if you have AO

// --- final ---
    float3 color = direct + ambient;
	// HDR tonemapping
    color = color / (color + float3(1.0, 1.0, 1.0));
	// gamma correct
    color = pow(color, float3((1.0 / 2.2), (1.0 / 2.2), (1.0 / 2.2)));
    
    //return float4(viewPos.zzz, 1.0);
    //return float4(color, 1.0);
    //return main1(albedo, N, P, LightDirection);
    //return main_debugViewZ(P);
    return float4(direct, 1.0f);
}

