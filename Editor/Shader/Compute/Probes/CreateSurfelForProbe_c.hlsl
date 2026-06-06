//Texture2DArray<float4> AlbedoTex : register(t0);
//Texture2DArray<float4> NormalTex : register(t1);
//Texture2DArray<float>  DepthTex  : register(t2);
//
//#include"../../include/GPUSurfel.hlsli"
//
//AppendStructuredBuffer<GPUSurfel> OutSurfels : register(u0);
//
//cbuffer ProbeSurfelCB : register(b0)
//{
//    float4x4 InvViewProj[6];
//
//    float3 ProbePosition;
//    float CaptureRadius;
//
//    uint FaceSize;
//    uint SourceProbeIndex;
//    uint ReversedZ;
//    uint Padding0;
//};
//
//
//
//float3 ReconstructWorldPosition(uint2 pixel, uint face, float depth)
//{
//    float u = ((float)pixel.x + 0.5f) / (float)FaceSize;
//    float v = ((float)pixel.y + 0.5f) / (float)FaceSize;
//
//    float2 ndc;
//    ndc.x = u * 2.0f - 1.0f;
//    ndc.y = 1.0f - v * 2.0f;
//
//    // D3D depth convention: z in 0..1.
//    float4 clipPos = float4(ndc.x, ndc.y, depth, 1.0f);
//
//    // Use this if your engine uses row-vector matrices:
//    float4 worldH = mul(clipPos, InvViewProj[face]);
//
//    // Use this instead if your engine uses column-vector matrices:
//    // float4 worldH = mul(InvViewProj[face], clipPos);
//
//    return worldH.xyz / max(worldH.w, 0.000001f);
//}
//
//float EstimateArea(float3 worldPos, float3 normal)
//{
//    float3 toProbe = ProbePosition - worldPos;
//    float dist = length(toProbe);
//
//    float3 dirToProbe = toProbe / max(dist, 0.0001f);
//
//    float cosTheta = max(abs(dot(normal, dirToProbe)), 0.2f);
//
//    float pixelSolidAngle =
//        4.0f * 3.14159265f / (6.0f * FaceSize * FaceSize);
//
//    return dist * dist * pixelSolidAngle / cosTheta;
//}
//
//[numthreads(8, 8, 1)]
//void main(uint3 id : SV_DispatchThreadID)
//{
//    uint x = id.x;
//    uint y = id.y;
//    uint face = id.z;
//
//    if (x >= FaceSize || y >= FaceSize || face >= 6)
//        return;
//
//    float depth = DepthTex.Load(int4(x, y, face, 0));
//
//    if (ReversedZ == 0)
//    {
//        if (depth >= 0.99999f)
//            return;
//    }
//    else
//    {
//        if (depth <= 0.00001f)
//            return;
//    }
//
//    float4 normalEncoded = NormalTex.Load(int4(x, y, face, 0));
//    float3 normal = normalEncoded;
//
//    float4 albedo = AlbedoTex.Load(int4(x, y, face, 0));
//
//    float3 worldPos = ReconstructWorldPosition(uint2(x, y), face, depth);
//
//    RawSurfelGPU s;
//    s.Position = worldPos;
//    s.Normal = normal;
//    s.Albedo = albedo.rgb;
//    s.Area = EstimateArea(worldPos, normal);
//    s.SourceProbeIndex = SourceProbeIndex;
//    s.SourceFace = face;
//    s.Pixel = uint2(x, y);
//    s.Padding = uint2(0, 0);
//    s.Radiance = 0.0;
//    s.Padding0 = 0.0;
//    OutSurfels.Append(s);
//}

void main()
{

}