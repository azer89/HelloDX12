
#include "ThreadIdToXYZ.hlsli"

TextureCube inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);

SamplerState defaultSampler : register(s0);

static const float PI = 3.141592653589;
static const float TWO_PI = PI * 2.0;
static const float HALF_PI = PI * 0.5;
static const float SAMPLE_DELTA = 0.05;

float3 Diffuse(float3 N)
{
    float3 diffuseColor = 0.0.xxx;

	// Tangent space calculation from origin point
    float3 up = float3(0.0, 1.0, 0.0);
    float3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));

    uint sampleCount = 0u;
    for (float phi = 0.0; phi < TWO_PI; phi += SAMPLE_DELTA)
    {
        for (float theta = 0.0; theta < HALF_PI; theta += SAMPLE_DELTA)
        {
			// Spherical to cartesian (in tangent space)
            float3 tangentSample = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            
			// Tangent space to world
            float3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
            
            float3 textureColor = inputTexture.SampleLevel(defaultSampler, sampleVec, 0).rgb;
            
            diffuseColor += textureColor * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    diffuseColor = PI * diffuseColor * (1.0 / float(sampleCount));

    return diffuseColor;
}

[numthreads(4, 4, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    uint outputWidth, outputHeight, outputDepth;
    outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);
    if (threadID.x >= outputWidth || threadID.y >= outputHeight)
    {
        return;
    }
    
    float3 scan = ThreadIdToXYZ(threadID, outputWidth, outputHeight);
    float3 direction = normalize(scan);
    float3 diffuseColor = Diffuse(direction);
    
    outputTexture[threadID] = float4(diffuseColor, 1.0);
}