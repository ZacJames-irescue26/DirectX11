

cbuffer DebugColors : register(b0)
{
    float4 Red;
    float4 Green;
    float4 Blue;
    int index;
};

float4 main() : SV_TARGET
{
    if (index == 0)
        return float4(Red.rgb, 0.2);
    if (index == 1)
        return float4(Green.rgb, 0.2);
    if (index == 2)
        return float4(Blue.rgb, 0.2);
    if (index == 3)
        return float4(Red.rgb + Green.rgb, 0.2);
    else
        return Red;
}