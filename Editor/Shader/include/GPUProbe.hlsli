#pragma once

struct GPUProbe
{
    float3 Position;
    float CaptureRadius;

    float3 AmbientCube0;
    float Pad0;

    float3 AmbientCube1;
    float Pad1;

    float3 AmbientCube2;
    float Pad2;

    float3 AmbientCube3;
    float Pad3;

    float3 AmbientCube4;
    float Pad4;

    float3 AmbientCube5;
    float Pad5;
};




float3 SampleAmbientCube(GPUProbe p, float3 n)
{
    float3 n2 = n * n;

    float3 result = 0.0f;

    result += (n.x > 0.0f ? p.AmbientCube0 : p.AmbientCube1) * n2.x;
    result += (n.y > 0.0f ? p.AmbientCube2 : p.AmbientCube3) * n2.y;
    result += (n.z > 0.0f ? p.AmbientCube4 : p.AmbientCube5) * n2.z;

    return result;
}

