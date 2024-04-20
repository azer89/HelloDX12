#include "LightData.hlsli"
#include "CameraData.hlsli"

struct VSInput
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 fragPosition : SV_POSITION;
    float4 worldPosition : POSITION0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer C0 : register(b0) {  CameraData camData; };
Texture2D albedoTexture : register(t0);
SamplerState albedoSampler : register(s0);
StructuredBuffer<LightData> lightDataArray : register(t1);

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.fragPosition = mul(input.position, camData.modelMatrix);
    result.worldPosition = result.fragPosition;
    result.fragPosition = mul(result.fragPosition, camData.viewMatrix);
    result.fragPosition = mul(result.fragPosition, camData.projectionMatrix);
    result.normal = mul(input.normal, ((float3x3) camData.modelMatrix));
    result.uv = input.uv.xy;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    uint len;
    uint stride;
    lightDataArray.GetDimensions(len, stride);
    
    float4 albedo = albedoTexture.Sample(albedoSampler, input.uv);
    float specular = albedo.a;
    
    for (uint i = 0; i < len; ++i)
    {
        LightData light = lightDataArray[i];
        
        // Diffuse
        float3 lightDir = normalize(light.position - input.worldPosition);
        float3 diffuse = max(dot(input.normal, lightDir), 0.0) * albedo.xyz * light.color.xyz;
    }
    
    // TODO Temporary
    LightData light = lightDataArray[len - 1];
    return light.color;
    
    //return g_texture.Sample(g_sampler, input.uv);
}
