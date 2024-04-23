
Texture2D srcImage : register(t0);
RWTexture2D<float4> dstImage : register(u0);

[numthreads(32, 32, 1)]
void CSMain(uint3 threadID : SV_DispatchThreadID)
{
    uint width, height;
    srcImage.GetDimensions(width, height);
    
    if (threadID.x <= width && threadID.y <= height)
    {
        float4 srcColor = srcImage[threadID.xy];
        dstImage[threadID.xy] = srcColor;
    }
}