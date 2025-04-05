TextureCube environmentMap : register(t0);
SamplerState HDRISamplerState : register(s0);

struct FS_INPUT
{
    float4 outPosition : SV_Position;
    float3 localPos : POSITION;
};

float4 main(FS_INPUT input) : SV_TARGET
{

    float3 N = normalize(input.localPos);

    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    static const int SAMPLE_COUNT = 32;
    static const float PI = 3.14159265359;
    static const float deltaTheta = 0.5 * PI / SAMPLE_COUNT;
    static const float deltaPhi = 2.0 * PI / SAMPLE_COUNT;

    float3 irradiance = float3(0.0, 0.0, 0.0);
    float totalWeight = 0.0;
    float sampleDelta = 0.025;
    float nrSamples = 0.0f;

    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
        {
            // spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += environmentMap.Sample(HDRISamplerState, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }


    irradiance = PI * irradiance * (1.0 / float(nrSamples));
    irradiance = saturate(irradiance);
    return float4(irradiance, 1.0);
}