
static const float LIGHT_INTENSITY = 1.75;
static const float LIGHT_FALLOFF = 0.1;

// Non-clustered version
float3 Radiance(
	float3 albedo,
	float3 N,
	float3 V,
	float3 F0,
    float3 worldPosition,
	float metallic,
	float roughness,
	float alphaRoughness,
	float NoV,
	LightData light)
{
    float3 Lo = float3(0.0, 0.0, 0.0);

    float3 L = normalize(light.position.xyz - worldPosition); // Incident light vector
	float3 H = normalize(V + L); // Halfway vector
	float NoH = max(dot(N, H), 0.0);
	float NoL = max(dot(N, L), 0.0);
	float HoV = max(dot(H, V), 0.0);
    float distance = length(light.position.xyz - worldPosition);

	// Physically correct attenuation
	//float attenuation = 1.0 / (distance * distance);

	// Hacky attenuation
    float attenuation = 1.0 / pow(distance, LIGHT_FALLOFF);

    float3 radiance = light.color.xyz * attenuation * LIGHT_INTENSITY;

	// Cook-Torrance BRDF
	float D = DistributionGGX(NoH, roughness);
	float G = GeometrySchlickGGX(NoL, NoV, alphaRoughness);
	float3 F = FresnelSchlick(HoV, F0);

	float3 numerator = D * G * F;
	float denominator = 4.0 * NoV * NoL + 0.0001; // + 0.0001 to prevent divide by zero
	float3 specular = numerator / denominator;

	// kS is equal to Fresnel
	float3 kS = F;
	// For energy conservation, the diffuse and specular light can't
	// be above 1.0 (unless the surface emits light); to preserve this
	// relationship the diffuse component (kD) should equal 1.0 - kS.
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
	// Multiply kD by the inverse metalness such that only non-metals 
	// have diffuse lighting, or a linear blend if partly metal (pure metals
	// have no diffuse light).
	kD *= 1.0 - metallic;

	float3 diffuse = Diffuse(kD * albedo);

	// Add to outgoing radiance Lo
	// Note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
	Lo += (diffuse + specular) * radiance * NoL;

	return Lo;
}