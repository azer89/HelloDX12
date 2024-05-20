
float3 NormalTBN(float3 textureNormal, float3 worldPos, float3 normal, float2 texCoord)
{
    float3 Q1 = ddx(worldPos);
    float3 Q2 = ddy(worldPos);
    float2 st1 = ddx(texCoord);
    float2 st2 = ddy(texCoord);

    float3 N = normalize(normal);
    float3 T = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    
    return normalize(mul(textureNormal, TBN));
}