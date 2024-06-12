#ifndef HELLO_DX12_USER_INPUT_DATA
#define HELLO_DX12_USER_INPUT_DATA

#include "ConstantBufferStructs.h"

struct UIData
{
public:
	bool imguiShow_ = true;

	// Mouse-related
	bool mouseFirstUse_ = true;
	bool mouseLeftPressed_ = false; // Left button pressed
	bool mouseLeftHold_ = false; // Left button hold

	// Mouse position at all times
	float mousePositionX_ = 0;
	float mousePositionY_ = 0;

	// Mouse position only when clicked
	float mousePressX_ = 0;
	float mousePressY_ = 0;

	CPBR constBufferPBR_ =
	{
		.lightIntensity = 1.75f,
		.baseReflectivity = 0.01f,
		.maxReflectionLod = 1.0f,
		.lightFalloff = 0.75f,
		.albedoMultipler = 0.1f
	};
};

#endif