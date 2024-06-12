
// Rename to CBPBR
struct CPBR
{
    float lightIntensity;
    float baseReflectivity;
    float maxReflectionLod; // TODO Implement IBL
    float lightFalloff; // Small --> slower falloff, Big --> faster falloff
    float albedoMultipler; // Show albedo color if the scene is too dark, default value should be zero
};