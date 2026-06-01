cbuffer buffer : register(b0)
{
    float4x4 wvpMatrix;
    float4x4 worldMatrix;
    float4x4 worldInvTransposeMatrix;
    float4x4 Bones[100];
};

struct VS_INPUT
{
    float3 inPos : POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inNormal : NORMAL;
    float3 inTangent : TANGENT;
    float3 inBiTangent : BITANGENT;
    uint4 inBoneIDs : BONEIDS;
    float4 inWeights : WEIGHTS;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float2 outTexCoord : TEXCOORD;
    float3 outNormal : NORMAL;
    float3 outTangent : TANGENT;
    float3 outBiTangent : BITANGENT;
    float3 outWorldPos : WORLD_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    float weightSum =
        input.inWeights.x +
        input.inWeights.y +
        input.inWeights.z +
        input.inWeights.w;

    matrix identityMatrix =
    {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    matrix skinMatrix = identityMatrix;

    if (weightSum > 0.00001f)
    {
        float4 weights = input.inWeights / weightSum;

        skinMatrix =
            weights.x * Bones[input.inBoneIDs.x] +
            weights.y * Bones[input.inBoneIDs.y] +
            weights.z * Bones[input.inBoneIDs.z] +
            weights.w * Bones[input.inBoneIDs.w];
    }

    float4 localPos = float4(input.inPos, 1.0f);
    float4 skinnedPos = mul(localPos, skinMatrix);
    
    //float4 skinnedPos = mul(float4(input.inPos, 1.0f), Bones[0]);



    float3 skinnedNormal =
        mul(float4(input.inNormal, 0.0f), skinMatrix).xyz;

    float3 skinnedTangent =
        mul(float4(input.inTangent, 0.0f), skinMatrix).xyz;

    float3 skinnedBiTangent =
        mul(float4(input.inBiTangent, 0.0f), skinMatrix).xyz;

    float4 worldPos = mul(skinnedPos, worldMatrix);

    output.outPosition = mul(skinnedPos, wvpMatrix);
    output.outTexCoord = input.inTexCoord;
    output.outWorldPos = worldPos.xyz;

    float3x3 normalMatrix = (float3x3)worldInvTransposeMatrix;

    output.outNormal = normalize(mul(skinnedNormal, normalMatrix));
    output.outTangent = normalize(mul(skinnedTangent, normalMatrix));
    output.outBiTangent = normalize(mul(skinnedBiTangent, normalMatrix));

    return output;
}