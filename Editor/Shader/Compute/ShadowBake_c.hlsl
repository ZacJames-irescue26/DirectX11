#include "../include/RayTraceFuncs.hlsli"
struct FlatNode
{
    float3 minBounds;
    bool IsLeaf;
    float3 maxBounds;
    bool IsBaseNode;

    int leftChild; // index of left child or -1
    int rightChild; // index of right child or -1

    int triOffset; // start index in flatTris
    int triCount; // number of triangles
};

struct Triangle
{
    float4 p0;
    float4 p1;
    float4 p2;
};


cbuffer LightCB : register(b0)
{
    float4x4 LightViewProj;
    float4x4 InvLightViewProj;
    float Bias;
    uint MapSize;
    uint NumNodes;
    uint NumTris;
    float pad0;
};

cbuffer BaseCB : register(b1)
{
    uint NumBases;
    float3 pad;
};
StructuredBuffer<FlatNode> nodes : register(t0);
StructuredBuffer<Triangle> tris : register(t1);
StructuredBuffer<uint> BaseRoots : register(t2);
RWTexture2D<float> ShadowMap : register(u0);
[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pix = DTid.xy;
    if (pix.x >= MapSize || pix.y >= MapSize)
        return;

    float2 uv = (pix + 0.5f) / MapSize;
    float2 ndc = uv * 2.0f - 1.0f;

    float4 clipN = float4(ndc, 0.0f, 1.0f);
    float4 clipF = float4(ndc, 1.0f, 1.0f);

    float4 worldNH = mul(InvLightViewProj, clipN);
    float4 worldFH = mul(InvLightViewProj, clipF);
    worldNH.xyz /= worldNH.w;
    worldFH.xyz /= worldFH.w;

    float3 P0 = worldNH.xyz;
    float3 dir = normalize(worldFH.xyz - P0);
    P0 += dir * Bias; 
    float3 invD = float3(
    abs(dir.x) > 1e-6 ? 1.0f / dir.x : 1e6,
    abs(dir.y) > 1e-6 ? 1.0f / dir.y : 1e6,
    abs(dir.z) > 1e-6 ? 1.0f / dir.z : 1e6
    );
    float tMin = Bias;
    float tMax = length(worldFH.xyz - P0) * 1.01;
    float closest = tMax;
    float debug = 0.0f;
    float globalClosest = tMax;
    for (int k = 0; k < NumBases; k++)
    {
        
       /*
        
        BaseNode1 - Mesh1 - search bvh for that node
        BaseNode2 - Mesh2 - search bvh for that node
        BaseNode3 - Mesh3 - search bvh for that node
        BaseNode4 - Mesh4 - search bvh for that node
 
        */
        
        float closest = tMax;
        int root = int(BaseRoots[k]);
        int stack[128];
        int sp = 0;
        stack[sp++] = root;
        while (sp > 0)
        {
            int idx = stack[--sp];
            if (idx < 0 || idx >= (int) NumNodes)
            {
                continue;
            }

            FlatNode n = nodes[idx];

            if (!RayIntersectsAABB(P0, invD, n.minBounds, n.maxBounds, tMin, closest))
            {
        
               
                continue;
            }
            if (n.leftChild < 0 && n.rightChild < 0)
            {
                for (int i = 0; i < n.triCount; ++i)
                {
                    Triangle T = tris[n.triOffset + i];
                    float t, u, v;
                    Ray R;
                    R.origin = P0;
                    R.dir = dir;
                    if (RayTriangleIntersect(R,
                    T.p0.xyz, T.p1.xyz, T.p2.xyz,
                    t, u, v)
                 && t < closest)
                    {
                        closest = t;
                        debug = 1.0;
                    }
                }

            }
            else
            {
                if (n.leftChild >= 0)
                    stack[sp++] = n.leftChild;
                if (n.rightChild >= 0)
                    stack[sp++] = n.rightChild;
            }
        }
        globalClosest = min(globalClosest, closest);
    }

    ShadowMap[pix] = globalClosest / tMax;
}