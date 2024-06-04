
#include "ThreadIdToXYZ.hlsli"

TextureCube inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);

SamplerState defaultSampler : register(s0);

static const float PI = 3.141592653589;

float3 Diffuse(float3 N)
{
    const float TWO_PI = PI * 2.0;
    const float HALF_PI = PI * 0.5;

    float3 diffuseColor = 0.0.xxx;

	// Tangent space calculation from origin point
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    float sampleDelta = 0.025;
    uint sampleCount = 0u;
    for (float phi = 0.0; phi < TWO_PI; phi += sampleDelta)
    {
        for (float theta = 0.0; theta < HALF_PI; theta += sampleDelta)
        {
			// Spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
			// Tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            //diffuseColor += texture(cubeMap, sampleVec).rgb * cos(theta) * sin(theta);
            diffuseColor += inputTexture.SampleLevel(defaultSampler, sampleVec, 0) * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    diffuseColor = PI * diffuseColor * (1.0 / float(sampleCount));

    return diffuseColor;
}

[numthreads(32, 32, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
}