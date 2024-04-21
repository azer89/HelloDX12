#include "LightData.hlsli"
#include "ModelData.hlsli"
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
cbuffer C1 : register(b1) {  ModelData modelData; };
Texture2D albedoTexture : register(t0);
SamplerState albedoSampler : register(s0);
StructuredBuffer<LightData> lightDataArray : register(t1);

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.fragPosition = mul(input.position, modelData.modelMatrix);
    result.worldPosition = result.fragPosition;
    result.fragPosition = mul(result.fragPosition, camData.viewMatrix);
    result.fragPosition = mul(result.fragPosition, camData.projectionMatrix);
    result.normal = mul(input.normal, ((float3x3) modelData.modelMatrix));
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
    
    const float LINEAR = 0.7f;
    const float QUADRATIC = 1.8f;
    float3 lighting = albedo.xyz * 0.01;
    float viewDir = normalize(camData.cameraPosition - input.worldPosition.xyz);
    for (uint i = 0; i < len; ++i)
    {
        LightData light = lightDataArray[i];
        
        // Diffuse
        float3 lightDir = normalize(light.position - input.worldPosition);
        float3 diffuse = max(dot(input.normal, lightDir), 0.0) * albedo.xyz * light.color.xyz;
        
        // Specular
        float3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(input.normal, halfwayDir), 0.0), 2.0);
        float3 specular = light.color.xyz * spec * albedo.xyz;
        
        // Attenuation
        float distance = length(light.position - input.worldPosition);
        //float attenuation = 1.0 / (1.0 + LINEAR * distance + QUADRATIC * distance * distance);
        float attenuation = 1.0 / pow(distance, 2.0f);
        
        diffuse *= attenuation;
        specular *= attenuation;

        lighting += diffuse + specular;
    }
    
    return float4(lighting, 1.0f);
}
