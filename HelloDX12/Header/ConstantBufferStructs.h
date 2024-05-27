#ifndef HELLO_DX12_CONSTANT_BUFFER_STRUCTS
#define HELLO_DX12_CONSTANT_BUFFER_STRUCTS

#include "glm/glm.hpp"

struct CCamera
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 cameraPosition;
};

struct CPBR
{
	float lightIntensity = 1.f;
	float baseReflectivity = 0.04f;
	float maxReflectionLod = 4.f;
	float lightFalloff = 1.0f; // Small --> slower falloff, Big --> faster falloff
	float albedoMultipler = 0.0f; // Show albedo color if the scene is too dark, default value should be zero
};

#endif