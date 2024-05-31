
#include "../PBR/Header.hlsli"
#include "Hammersley.hlsli"

static const uint SAMPLE_COUNT = 1024;
static const float INV_SAMPLE_COUNT = 1.0 / float(SAMPLE_COUNT);

RWTexture2D<float2> LUT : register(u0);

float2 BRDF(float NoV, float roughness)
{
	// Normal always points along z-axis
    const float3 N = float3(0.0, 0.0, 1.0);
    float3 V = float3(sqrt(1.0 - NoV * NoV), 0.0, NoV);

    float2 lutValue = float2(0.0);
    for (uint i = 0; i < SAMPLE_COUNT; i++)
    {
        float2 Xi = Hammersley(i, SAMPLE_COUNT);
        float3 H = ImportanceSampleGGX(Xi, N, roughness);
        float3 L = 2.0 * dot(V, H) * H - V;

        float dotNL = max(dot(N, L), 0.0);

        if (dotNL > 0.0)
        {
            float dotNV = max(dot(N, V), 0.0);
            float dotVH = max(dot(V, H), 0.0);
            float dotNH = max(dot(H, N), 0.0);

            float alpha = AlphaIBL(roughness);
            float G = GeometrySchlickGGX(dotNL, dotNV, alpha);
            float G_Vis = (G * dotVH) / (dotNH * dotNV);
            float Fc = pow(1.0 - dotVH, 5.0);
            lutValue += float2(
				(1.0 - Fc) * G_Vis, // NoV
				Fc * G_Vis); // Roughness
        }
    }
    return lutValue / float(SAMPLE_COUNT);
}

[numthreads(32, 32, 1)]
void CSMain(uint2 threadID : SV_DispatchThreadID)
{
}