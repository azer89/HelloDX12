


struct VSInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer Constants : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProjection;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

#include "LightData.hlsl"

StructuredBuffer<LightData> lightDataArray : register(t1);

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.position = mul(input.position, mWorld);
    result.position = mul(result.position, mView);
    result.position = mul(result.position, mProjection);
    
    result.normal = mul(input.normal, ((float3x3) mWorld));
    
    result.uv = input.uv.xy;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    uint l;
    uint s;
    lightDataArray.GetDimensions(l, s);
    
    LightData light = lightDataArray[l - 1];
    return light.color;
    
    //return g_texture.Sample(g_sampler, input.uv);
}
