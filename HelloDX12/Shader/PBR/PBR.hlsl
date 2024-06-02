#include "../VertexData.hlsli"
#include "../LightData.hlsli"
#include "../ModelData.hlsli"
#include "../MeshData.hlsli"
#include "../CameraData.hlsli"

#include "Header.hlsli"
#include "NormalTBN.hlsli"
#include "Radiance.hlsli"
#include "CPBR.hlsli"

struct VSInput
{
    uint vertexID : SV_VertexID;
};

struct PSInput
{
    float4 fragPosition : SV_POSITION;
    float4 worldPosition : POSITION0;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

cbuffer RootConstants : register(b0)
{
    // This is equivalent to gl_DrawID or gl_BaseInstance 
    uint meshIndex;
};

cbuffer ConstantBuffer1 : register(b1)
{
    CameraData camData;
};

cbuffer ConstantBuffer2 : register(b2)
{
    ModelData modelData;
};

cbuffer ConstantBuffer3 : register(b3)
{
    CPBR cPBR;
};

StructuredBuffer<VertexData> vertexDataArray : register(t0);
StructuredBuffer<uint> indexArray : register(t1);
StructuredBuffer<MeshData> meshDataArray : register(t2);
StructuredBuffer<LightData> lightDataArray : register(t3);

Texture2D brdfLut : register(t4);
Texture2D allTextures[] : register(t5); // Unbounded array

SamplerState defaultSampler : register(s0);

PSInput VSMain(VSInput input)
{
    // Vertex pulling
    MeshData m = meshDataArray[meshIndex];
    uint vertexIndex = indexArray[input.vertexID + m.indexOffset] + m.vertexOffset;
    VertexData v = vertexDataArray[vertexIndex];
    
    PSInput result;
    
    result.fragPosition = mul(float4(v.position, 1.0), modelData.modelMatrix);
    result.worldPosition = result.fragPosition;
    result.fragPosition = mul(result.fragPosition, camData.viewMatrix);
    result.fragPosition = mul(result.fragPosition, camData.projectionMatrix);
    result.normal = mul(v.normal, ((float3x3) modelData.modelMatrix));
    result.uv = v.uv.xy;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    MeshData m = meshDataArray[meshIndex];
    float4 albedo4 = allTextures[NonUniformResourceIndex(m.albedo)].Sample(defaultSampler, input.uv);
    
    if (albedo4.a < 0.5)
    {
        discard;
    }
    
    // PBR textures
    float3 albedo = albedo4.xyz;
    float3 emissive = allTextures[NonUniformResourceIndex(m.emissive)].Sample(defaultSampler, input.uv).rgb;
    float3 texNormalValue = allTextures[NonUniformResourceIndex(m.normal)].Sample(defaultSampler, input.uv).rgb * 2.0 - 1.0;
    float metallic = allTextures[NonUniformResourceIndex(m.metalness)].Sample(defaultSampler, input.uv).b;
    float roughness = allTextures[NonUniformResourceIndex(m.roughness)].Sample(defaultSampler, input.uv).g;
    float ao = allTextures[NonUniformResourceIndex(m.ao)].Sample(defaultSampler, input.uv).r;
    
    // PBR
    float alphaRoughness = AlphaDirectLighting(roughness);
    float3 N = NormalTBN(texNormalValue, input.worldPosition.xyz, input.normal, input.uv);
    float3 V = normalize(camData.cameraPosition - input.worldPosition.xyz);
    float NoV = max(dot(N, V), 0.0);
    float3 F0 = cPBR.baseReflectivity.xxx;
    F0 = lerp(F0, albedo, metallic);
    float3 Lo = albedo * cPBR.albedoMultipler;
    
    uint len;
    uint stride;
    lightDataArray.GetDimensions(len, stride);
    for (uint i = 0; i < len; ++i)
    {
        LightData light = lightDataArray[i];
        Lo += Radiance(
            albedo,
            N,
            V,
            F0,
            input.worldPosition.xyz,
            metallic,
            roughness,
            alphaRoughness,
            NoV,
            cPBR.lightIntensity,
            cPBR.lightFalloff,
            light);
    }
    
    float3 color = emissive + Lo;
    return float4(color, 1.0f);
}