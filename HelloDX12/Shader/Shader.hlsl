struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 Normal : NORMAL;
};

cbuffer Constants : register(b0)
{
    float4x4 mWorld;
    float4x4 mView;
    float4x4 mProjection;
};

Texture2D g_texture : register(t0);
SamplerState g_sampler : register(s0);

PSInput VSMain(float4 position : POSITION, float4 uv : TEXCOORD)
{
    PSInput result;

    result.position = position;
    result.uv = uv;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return g_texture.Sample(g_sampler, input.uv);
}
