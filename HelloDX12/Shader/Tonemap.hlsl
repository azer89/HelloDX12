
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
    result.fragPosition = float4(x - 1.0, y - 1.0, 0, 1);
    
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float4 color4 = srcImage.Sample(srcSampler, input.uv);
    
    return float4(color4.xyz, 1.0f);
}