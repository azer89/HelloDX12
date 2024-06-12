
/*
Downsample shader for generating mipmap
*/

Texture2D<float4> srcTexture : register(t0);
RWTexture2DArray<float4> dstTexture : register(u0); // Array for cubemap
SamplerState defaultSampler : register(s0);

cbuffer CB : register(b0)
{
    // 1.0 / dimension
    float2 texelSize; 
}

[numthreads(8, 8, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
	// The center between the 4 pixels used for the mipmap
    float2 texcoords = texelSize * (threadID.xy + 0.5);

	// The samplers linear interpolation will mix the four pixel values to the new pixels color
    float4 color = srcTexture.SampleLevel(defaultSampler, texcoords, 0);

	// Write the final color into the destination texture
    dstTexture[threadID] = color;
}