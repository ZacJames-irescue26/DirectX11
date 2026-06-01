cbuffer lightBuffer : register(b0)
{
	float3 ambientLightColor;
	float ambientLightStrength;

	float3 dynamicLightColor;
	float dynamicLightStrength;
	float3 dynamicLightPosition;
	
	
}
cbuffer CamerInfo : register(b1)
{
    float3 cameraPosition;
}
struct PS_INPUT
{
	float4 inPosition : SV_POSITION;
	float2 inTexCoord : TEXCOORD;
	float3 inNormal : NORMAL;
	float3 inWorldPos : WORLD_POSITION;
};

Texture2D DiffuseTexture : TEXTURE: register(t0);
Texture2D NormalTexture : TEXTURE: register(t1);
Texture2D MetallicTexture : TEXTURE : register(t2);
Texture2D RoughnessTexture : TEXTURE : register(t3);


SamplerState objSamplerState : SAMPLER: register(s0);

const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anyways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
float3 getNormalFromMap(float2 TexCoords, float3 WorldPos, float3 Normal)
{
    float3 tangentNormal = NormalTexture.Sample(objSamplerState, TexCoords).xyz * 2.0 - 1.0;

    float3 Q1 = ddx(WorldPos);
    float3 Q2 = ddy(WorldPos);
    float2 st1 = ddx(TexCoords);
    float2 st2 = ddy(TexCoords);

    float3 N = normalize(Normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);

    return normalize(mul(TBN, tangentNormal));
}
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
float4 main(PS_INPUT input) : SV_TARGET
{
    float3 albedo = pow(DiffuseTexture.Sample(objSamplerState, input.inTexCoord).xyz, float3(2.2, 2.2, 2.2));
	float metallic = MetallicTexture.Sample(objSamplerState, input.inTexCoord).x;
    float roughness = RoughnessTexture.Sample(objSamplerState, input.inTexCoord).x;

	//float ao = texture(aoMap, TexCoords).r;
    float ao = 1.0f;
    float3 N = NormalTexture.Sample(objSamplerState, input.inTexCoord);
    //float3 N = getNormalFromMap(input.inTexCoord, input.inWorldPos, input.inNormal);
	float3 V = normalize(cameraPosition - input.inWorldPos);

	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
	// of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
	float3 F0 = float3(0.04, 0.04, 0.04);
	F0 = lerp(F0, albedo, metallic);

	// reflectance equation
	float3 Lo = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 1; ++i)
	{
        float3 WorldPos = input.inWorldPos;
		// calculate per-light radiance
		float3 L = normalize(dynamicLightPosition - WorldPos);
		float3 H = normalize(V + L);
        float distance = length(dynamicLightPosition - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		float3 radiance = dynamicLightColor * attenuation;

		// Cook-Torrance BRDF
		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		float3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

		float3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero
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
		kD *= 1.0 - metallic;

		// scale light by NdotL
		float NdotL = max(dot(N, L), 0.0);

		// add to outgoing radiance Lo
		Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	}

	// ambient lighting (note that the next IBL tutorial will replace 
	// this ambient lighting with environment lighting).
	float3 ambient = albedo * ao;

	float3 color = ambient;

	// HDR tonemapping
	color = color / (color + float3(1.0, 1.0, 1.0));
	// gamma correct
    color = pow(color, float3((1.0 / 2.2), (1.0 / 2.2), (1.0 / 2.2)));

	return float4(color, 1.0);
}