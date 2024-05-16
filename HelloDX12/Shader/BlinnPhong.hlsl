#include "VertexData.hlsli"
#include "LightData.hlsli"
#include "ModelData.hlsli"
#include "MeshData.hlsli"
#include "CameraData.hlsli"

struct PSInput
{
    float4 fragPosition : SV_POSITION;
    float4 worldPosition : POSITION0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer C0 : register(b0) {  CameraData camData; };
cbuffer C1 : register(b1) {  ModelData modelData; };

StructuredBuffer<VertexData> vertexDataArray : register(t0);
StructuredBuffer<uint> indexArray : register(t1);
StructuredBuffer<MeshData> meshDataArray : register(t2);
StructuredBuffer<LightData> lightDataArray : register(t3);

Texture2D allTextures[] : register(t4); // Unbounded array

SamplerState defaultSampler : register(s0);

PSInput VSMain(uint vertexID : SV_VertexID, 
               uint instanceID : SV_InstanceID)
{ 
    // Vertex pulling
    uint vertexIndex = indexArray[vertexID];
    VertexData input = vertexDataArray[vertexIndex];
    
    PSInput result;
    
    result.fragPosition = mul(float4(input.position, 1.0), modelData.modelMatrix);
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
    
    // TODO Only one mesh for now
    MeshData m = meshDataArray[0];
    uint texIndex = m.albedo;
    float4 albedo = allTextures[NonUniformResourceIndex(texIndex)].Sample(defaultSampler, input.uv);
    
    // Albedo component
    float3 lighting = albedo.xyz * 0.01;
    
    float3 viewDir = normalize(camData.cameraPosition - input.worldPosition.xyz);
    for (uint i = 0; i < len; ++i)
    {
        LightData light = lightDataArray[i];
        float3 lightDir = normalize(light.position - input.worldPosition).xyz;
        
        // Diffuse component
        float3 diffuse = max(dot(input.normal, lightDir), 0.0) * albedo.xyz * light.color.xyz;
        
        // Specular component
        float3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(input.normal, halfwayDir), 0.0), 8.0);
        float3 specular = light.color.xyz * spec * albedo.xyz;
        
        // Attenuation
        float distance = length(light.position - input.worldPosition);
        float attenuation = 1.0 / pow(distance, 2.0);
        
        diffuse *= attenuation;
        specular *= attenuation;

        lighting += diffuse + specular;
    }
    
    return float4(lighting, 1.0f);
}
