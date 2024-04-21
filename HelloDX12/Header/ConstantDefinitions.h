#ifndef HELLO_DX12_CONSTANT_BUFFER_DEFINITIONS
#define HELLO_DX12_CONSTANT_BUFFER_DEFINITIONS

#include "glm/glm.hpp"

struct CCamera
{
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;
	glm::vec3 cameraPosition;
};

struct CModel
{
	glm::mat4 modelMatrix;
};

#endif