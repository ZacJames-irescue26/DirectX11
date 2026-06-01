
#define TILE_SIZEPIX 16
//GBUffer
Texture2D<float4> gPositionBuffer : register(t0);
Texture2D<float4> gNormalBuffer : register(t1);
Texture2D<float4> gAlbedoBuffer : register(t2);
Texture2D DepthTexture : register(t3);
SamplerState linearSampler : register(s0);

//Surfels
struct Surfel
{
    float4 position;
    float4 normal;
    float4 albedo;
    float3 indirectRadiance;
    float radius;
};

RWStructuredBuffer<Surfel> gSurfels : register(u0);
RWByteAddressBuffer gSurfelCounter : register(u1);
RWTexture2D<uint> gTileCoverage : register(u2); 

cbuffer CameraData : register(b0)
{
    uint screenWidth;
    uint screenHeight;
    float padding;
    float padding1; 
    float4x4 viewProj;
    float2 screenSize;
    uint tileCountX;
    uint tileCountY;
   
};

cbuffer InvViewProj : register(b1)
{
    float4x4 InvViewProj;
}

float3 ReconstructWorldPos(float2 uv)
{
    float d = DepthTexture.SampleLevel(linearSampler, uv, 0).r;
    
    float2 ndcXY = uv * 2.0f - 1.0f;
 
    ndcXY.y = -ndcXY.y;
    float ndcZ = d * 2.0f - 1.0f; 

    float4 clipPos = float4(ndcXY, ndcZ, 1.0f);
    float4 world = mul(clipPos, InvViewProj);
    return world.xyz / world.w;
}

// === Hash Function ===
uint wang_hash(uint seed)
{
    seed = (seed ^ 61) ^ (seed >> 16);
    seed *= 9;
    seed = seed ^ (seed >> 4);
    seed *= 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}
float hash01(uint2 coord)
{
    uint h = wang_hash(coord.x + coord.y * 4096);
    return (h & 0x00FFFFFF) / 16777216.0;
}

[numthreads(16, 16, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
    if (GTid.x != 0 || GTid.y != 0)
        return; // Only one thread per tile acts
    uint2 TileID = Gid.xy;
    
    if (TileID.x >= screenWidth || TileID.y >= screenHeight)
        return;
   
    uint currentSurfelCount;
    gSurfelCounter.Load(0, currentSurfelCount);
    if (currentSurfelCount >= 10000)
        return;
    
    float tileCoverage = clamp(gTileCoverage[TileID] / 100.0f, 0.0f, 1.0f);
    float rand = hash01(TileID);
    if (rand > (1.0 - tileCoverage))
        return;
   
    uint2 centerPixel = TileID * TILE_SIZEPIX + TILE_SIZEPIX / 2;
   
    if (centerPixel.x >= screenWidth || centerPixel.y >= screenHeight)
        return;
   
    float2 uv = float2(centerPixel) / float2(screenWidth, screenHeight);
    
    float3 position = ReconstructWorldPos(uv);
    //float3 position = float3(1.0, 1.0, 1.0);
    if (all(position == 0))
        return;
    float3 normal = gNormalBuffer.SampleLevel(linearSampler, uv, 0).xyz;
   
    
    float3 albedo = gAlbedoBuffer.SampleLevel(linearSampler, uv, 0).xyz;
    
    Surfel surf;
    
    surf.position = float4(position, 1.0f);
    surf.normal = float4(normalize(normal), 0.0f);
    surf.albedo = float4(albedo, 1.0f);
    surf.radius = 1;
    surf.indirectRadiance = float3(0.0, 0.0, 0.0);
    uint index;
    gSurfelCounter.InterlockedAdd(0, 1, index);
    gSurfels[index] = surf;
    
    // === Accumulate tile coverage locally ===
    float4 clip = mul(viewProj, float4(surf.position.xyz, 1.0));
    float2 ndc = clip.xy / clip.w;
    float2 screenPx = (ndc * 0.5 + 0.5) * screenSize;

    uint2 centerTile = uint2(screenPx / TILE_SIZEPIX);
    int tileRadius = int(ceil(surf.radius / TILE_SIZEPIX));


    uint2 tilePixelMin = uint2(max(0, screenPx.x - surf.radius) / TILE_SIZEPIX, max(0, screenPx.y - surf.radius) / TILE_SIZEPIX);
    uint2 tilePixelMax = uint2(min(screenPx.x + surf.radius, screenSize.x - 1) / TILE_SIZEPIX, min(screenPx.y + surf.radius, screenSize.y - 1) / TILE_SIZEPIX);

[loop]
    for (uint y = tilePixelMin.y; y <= tilePixelMax.y; y++)
    {
        for (uint x = tilePixelMin.x; x <= tilePixelMax.x; x++)
        {
            float2 tileCenterPx = float2(x * TILE_SIZEPIX + TILE_SIZEPIX / 2, y * TILE_SIZEPIX + TILE_SIZEPIX / 2);
            float dist = length(screenPx - tileCenterPx);
            float coveragePercent = saturate(1.0 - dist / surf.radius);
            uint coverageValue = uint(coveragePercent * 100.0f);
            InterlockedAdd(gTileCoverage[uint2(x, y)], coverageValue);
        }
    }

    
    
}