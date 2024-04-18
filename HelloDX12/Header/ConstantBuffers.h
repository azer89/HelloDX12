#ifndef HELLO_DX12_CONSTANT_BUFFER_DEFINITIONS
#define HELLO_DX12_CONSTANT_BUFFER_DEFINITIONS

#include "glm/glm.hpp"

struct CBMVP
{
	glm::mat4 worldMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
};

#endif