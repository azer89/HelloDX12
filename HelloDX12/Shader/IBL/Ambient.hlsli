
float3 Ambient(
	float3 albedo,
	float3 F0,
	float3 N,
	float3 V,
    float maxReflectionLod,
	float metallic,
	float roughness,
	float ao,
	float NoV)
{
	// Ambient lighting (we now use IBL as the ambient term)
    float3 F = FresnelSchlickRoughness(NoV, F0, roughness);

    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    //float3 irradiance = texture(diffuseMap, N).rgb;
    float3 irradiance = diffuseMap.Sample(defaultSampler, N).xyz;
    
    float3 diffuse = irradiance * albedo;

	// Sample both the pre-filter map and the BRDF lut and combine them together as
	// per the Split-Sum approximation to get the IBL specular part.
    float3 R = reflect(-V, N);
    float3 prefilteredColor = specularMap.SampleLevel(defaultSampler, R, roughness * maxReflectionLod).rgb;
    float2 brdf = brdfLUT.Sample(brdfLutSampler, float2(NoV, roughness)).rg;
    
    float3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    return (kD * diffuse + specular) * ao;
}
