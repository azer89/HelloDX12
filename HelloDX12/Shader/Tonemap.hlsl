
struct VSInput
{
    uint vertexIndex : SV_VertexID;
};

struct PSInput
{
    float4 fragPosition : SV_POSITION;
    float2 uv : TEXCOORD;
};

Texture2D srcImage : register(t0);
SamplerState srcSampler : register(s0);

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    float x = float((input.vertexIndex & 1) << 2);
    float y = float((input.vertexIndex & 2) << 1);
    result.uv = float2(x * 0.5, y * 0.5);
    result.fragPosition = float4(x - 1.0, 1.0 - y, 0, 1);
    
    return result;
}

float3 Reinhard(float3 color)
{
    const float pureWhite = 1.0;

    float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));
    float mappedLuminance =
		(luminance * (1.0 + luminance / (pureWhite * pureWhite))) / (1.0 + luminance);

	// Scale color by ratio of average luminances
    return (mappedLuminance / luminance) * color;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color4 = srcImage.Sample(srcSampler, input.uv);
    
    float3 tonemapped = Reinhard(color4.xyz);
    
     // Gamma correction
    float gamma = 1.0 / 2.2;
    tonemapped = pow(tonemapped, gamma);
    
    return float4(tonemapped.xyz, 1.0f);
}