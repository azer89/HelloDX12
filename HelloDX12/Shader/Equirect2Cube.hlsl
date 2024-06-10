
static const float PI = 3.141592;
static const float TwoPI = 2 * PI;

Texture2D inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);
SamplerState defaultSampler : register(s0);

float3 GetSamplingVector(uint3 threadID)
{
    float outputWidth;
    float outputHeight;
    float outputDepth;
    outputTexture.GetDimensions(outputWidth, outputHeight, outputDepth);

    float2 st = threadID.xy / float2(outputWidth, outputHeight);
    float2 uv = 2.0 * float2(st.x, 1.0 - st.y) - float2(1.0, 1.0);
    
    float3 ret;
    switch (threadID.z)
    {
        case 0:
            ret = float3(1.0, uv.y, -uv.x);
            break;
        case 1:
            ret = float3(-1.0, uv.y, uv.x);
            break;
        case 2:
            ret = float3(uv.x, 1.0, -uv.y);
            break;
        case 3:
            ret = float3(uv.x, -1.0, uv.y);
            break;
        case 4:
            ret = float3(uv.x, uv.y, 1.0);
            break;
        case 5:
            ret = float3(-uv.x, uv.y, -1.0);
            break;
    }
    return normalize(ret);
}

[numthreads(32, 32, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    float3 v = GetSamplingVector(threadID);
	
	// Cartesian to spherical coordinates
    float phi = atan2(v.z, v.x);
    float theta = acos(v.y);

    // TODO
    // normalize is used to prevent extreme values found in blight light sources
    float4 color = normalize(inputTexture.SampleLevel(defaultSampler, float2(phi / TwoPI, theta / PI), 0));
    
    outputTexture[threadID] = color;
}