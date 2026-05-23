#include "../include/RayTraceFuncs.hlsli"
struct FlatNode
{
    float3 minBounds;
    uint IsLeaf;
    float3 maxBounds;
    uint IsBaseNode;

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
    float3 LightDir;
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
Texture2D PositionTexture : register(t3);
RWTexture2D<float> ShadowMap : register(u0);

float ComputeShadow(in float3 P0, in float3 dir)
{
    float tMin = Bias;
    float tMax = 1000;
    float3 invD = 1.0 / max(abs(dir), 1e-6);
    bool hit = false;
    float hitT = tMax;

    for (int k = 0; k < NumBases; k++)
    {
        int root = int(BaseRoots[k]);
        int stack[128];
        int sp = 0;
        stack[sp++] = root;

        while (sp > 0)
        {
            int idx = stack[--sp];
            if (idx < 0 || idx >= int(NumNodes))
                continue;

            FlatNode n = nodes[idx];

            if (!RayIntersectsAABB(P0, invD, n.minBounds, n.maxBounds, tMin, hitT))
                continue;

            if (n.leftChild < 0 && n.rightChild < 0)
            {
                for (int i = 0; i < n.triCount; ++i)
                {
                    Triangle T = tris[n.triOffset + i];
                    float t, u, v;
                    Ray R = { P0, dir };
                    if (RayTriangleIntersect(R, T.p0.xyz, T.p1.xyz, T.p2.xyz, t, u, v) && t >= tMin && t < hitT)
                    {
                        hitT = t;
                        hit = true;
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
    }

    return hit ? 1.0f : 0.0f;
}


[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint2 pix = DTid.xy;
    if (pix.x >= 1920/4 || pix.y >= 1080/4)
        return;
    float4 posW = PositionTexture.Load(int3(pix, 0));
    /*if (posW.w == 0)   
    {
        ShadowMap[pix] = 0;
        return;
    }*/

    float3 dir = normalize(-LightDir);
    float3 P0 = posW.xyz + dir * Bias;

    ShadowMap[pix] = ComputeShadow(P0, dir);
}