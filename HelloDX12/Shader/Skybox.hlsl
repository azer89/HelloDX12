#include "CameraData.hlsli"

struct PSInput
{
    float3 localPosition : POSITION;
    float4 pixelPosition : SV_POSITION;
};

cbuffer C0 : register(b0) { CameraData camData; };

TextureCube envTexture : register(t0);
SamplerState defaultSampler : register(s0);

static const float3 CUBE_POS[8] =
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

static const int CUBE_INDICES[36] =
{
    0, 1, 2, 2, 3, 0, // Front
    1, 5, 6, 6, 2, 1, // Right
    7, 6, 5, 5, 4, 7, // Back
    4, 0, 3, 3, 7, 4, // Left
    4, 5, 1, 1, 0, 4, // Bottom
    3, 2, 6, 6, 7, 3  // Top
};

PSInput VSMain(uint vertexID : SV_VertexID)
{
    PSInput outValue;
    
    int idx = CUBE_INDICES[vertexID];
    float4 pos4 = float4(CUBE_POS[idx], 1.0);
    
    // NOTE Set depth compare in PSO
    float4 posTransform = mul(pos4, camData.viewMatrix);
    posTransform = mul(posTransform, camData.projectionMatrix);
    posTransform = posTransform.xyww;
    
    outValue.pixelPosition = posTransform;
    outValue.localPosition = pos4.xyz;
    
    return outValue;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 envVector = normalize(input.localPosition);
    return envTexture.SampleLevel(defaultSampler, envVector, 0);
}