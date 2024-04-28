#include "CameraData.hlsli"

struct PSInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

const float3 CUBE_POS[8] =
{
    float3(-1.0, -1.0, 1.0),
    float3( 1.0, -1.0, 1.0),
    float3( 1.0,  1.0, 1.0),
    float3(-1.0,  1.0, 1.0),

    float3(-1.0, -1.0, -1.0),
    float3( 1.0, -1.0, -1.0),
    float3( 1.0,  1.0, -1.0),
    float3(-1.0,  1.0, -1.0)
};

const int CUBE_INDICES[36] =
{
    0, 1, 2, 2, 3, 0, // Front
    1, 5, 6, 6, 2, 1, // Right
    7, 6, 5, 5, 4, 7, // Back
    4, 0, 3, 3, 7, 4, // Left
    4, 5, 1, 1, 0, 4, // Bottom
    3, 2, 6, 6, 7, 3 // Top
};

PSInput VSMain()
{
    PSInput outValue;
    
    return outValue;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(1.0, 1.0, 1.0, 1.0);

}