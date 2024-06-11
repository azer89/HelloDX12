#ifndef HELLO_DX12_CONSTANT_BUFFER_STRUCTS
#define HELLO_DX12_CONSTANT_BUFFER_STRUCTS

#include "glm/glm.hpp"

// Rename to CBCamera
struct CCamera
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 cameraPosition;
};

// Rename to CBPBR
struct CPBR
{
	float lightIntensity;
	float baseReflectivity;
	float maxReflectionLod; // TODO Implement IBL
	float lightFalloff; // Small --> slower falloff, Big --> faster falloff
	float albedoMultipler; // Show albedo color if the scene is too dark, default value should be zero
};

#endif