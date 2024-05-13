#ifndef HELLO_DX12_CONSTANT_BUFFER_DEFINITIONS
#define HELLO_DX12_CONSTANT_BUFFER_DEFINITIONS

#include "glm/glm.hpp"

// TODO Rename file to ConstantBufferStructs.h

struct CCamera
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 cameraPosition;
};

#endif