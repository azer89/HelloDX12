
#include "ThreadIdToXYZ.hlsli"
#include "Hammersley.hlsli"
#include "../PBR/Header.hlsli"

cbuffer SpecularMapFilterSettings : register(b0)
{
    float roughness;
};

TextureCube inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);

SamplerState defaultSampler : register(s0);

static const uint SAMPLE_COUNT = 1024;

float3 Specular(float3 N)
{
	// Make the simplifying assumption that V equals R equals the normal 
    float3 R = N;
    float3 V = R;

    //float2 texelSize = 1.0 / textureSize(cubeMap, 0); // TODO
    //float saTexel = 4.0 * PI / (6.0 * texelSize.x * texelSize.x);
    // Get input cubemap dimensions at zero mipmap level.
    float inputWidth, inputHeight, inputLevels;
    inputTexture.GetDimensions(0, inputWidth, inputHeight, inputLevels);
    float saTexel = 4.0 * PI / (6.0 * inputWidth * inputHeight);

    float3 specularColor = float3(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
		// Generates a sample vector that's biased towards
		// the preferred alignment direction (importance sampling).
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = normalize(2.0 * dot(V, H) * H - V);

        float NoL = max(dot(N, L), 0.0);

        if (NoL > 0.0)
        {
            float NoH = max(dot(N, H), 0.0);
            float HoV = max(dot(H, V), 0.0);

			// Sample from the environment's mip level based on roughness/pdf
            float D = DistributionGGX(NoH, roughness); // Trowbridge-Reitz GGX
            float pdf = D * NoH / (4.0 * HoV) + 0.0001;
            float saSample = 1.0 / (float(SAMPLE_COUNT) * pdf + 0.0001);
            float mipLevel = roughness == 0.0 ? 0.0 : 0.5 * log2(saSample / saTexel);

            //specularColor += textureLod(cubeMap, L, mipLevel).rgb * NoL; // TODO
            // color  += inputTexture.SampleLevel(defaultSampler, Li, mipLevel).rgb * cosLi;
            specularColor += inputTexture.SampleLevel(defaultSampler, L, mipLevel).rgb * NoL;
            totalWeight += NoL;
        }
    }

    specularColor = specularColor / totalWeight;

    return specularColor;
}

[numthreads(32, 32, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
}