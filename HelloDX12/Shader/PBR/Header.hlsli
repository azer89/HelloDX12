#define PI 3.1415926535897932384626433832795

// Specular D
// Trowbridge-Reitz GGX that models the distribution of microfacet normal.
float DistributionGGX(float NoH, float roughness)
{
    float alpha = roughness * roughness; // Disney remapping
    float alpha2 = alpha * alpha;
    float NoH2 = NoH * NoH;

    float nominator = alpha2;
    float denominator = (NoH2 * (alpha2 - 1.0) + 1.0);
    denominator = PI * denominator * denominator;

    return nominator / denominator;
}

// Roughness remapping for IBL lighting
float AlphaIBL(float roughness)
{
    float alpha = (roughness * roughness) / 2.0;
    return alpha;
}

// Roughness remapping for direct lighting (See Brian Karis's PBR Note)
float AlphaDirectLighting(float roughness)
{
    float r = (roughness + 1.0);
    float alpha = (r * r) / 8.0;
    return alpha;
}

// Specular G
// Geometry function that describes the self-shadowing property of the microfacets.
// When a surface is relatively rough, the surface's microfacets can overshadow other
// microfacets reducing the light the surface reflects.
float GeometrySchlickGGX(float NoL, float NoV, float alpha)
{
    float GL = NoL / (NoL * (1.0 - alpha) + alpha);
    float GV = NoV / (NoV * (1.0 - alpha) + alpha);
    return GL * GV;
}

// Specular F
// The Fresnel equation describes the ratio of surface reflection at different surface angles.
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    float oneMinRoughness = 1.0 - roughness;
    float3 oneMinRoughness3 = float3(oneMinRoughness, oneMinRoughness, oneMinRoughness);
    return F0 + (max(oneMinRoughness3, F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness; // Roughness remapping

    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

	// From spherical coordinates to cartesian coordinates - halfway vector
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;

	// From tangent-space H vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);

    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

// Basic Lambertian diffuse
float3 Diffuse(float3 albedo)
{
    return albedo / PI;
}