
static const float PI = 3.141592;
static const float TwoPI = 2 * PI;

Texture2D inputTexture : register(t0);
RWTexture2DArray<float4> outputTexture : register(u0);
SamplerState defaultSampler : register(s0);

[numthreads(32, 32, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
}