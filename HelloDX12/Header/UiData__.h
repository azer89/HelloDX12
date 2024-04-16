#ifndef HELLO_DX12_USER_INPUT_DATA
#define HELLO_DX12_USER_INPUT_DATA

struct UIData
{
public:
	// Mouse-related
	bool mouseFirstUse_ = true;
	bool mouseLeftPressed_ = false; // Left button pressed
	bool mouseLeftHold_ = false; // Left button hold

	// Mouse position at all times
	float mousePositionX = 0;
	float mousePositionY = 0;

	// Mouse position only when clicked
	float mousePressX = 0;
	float mousePressY = 0;
};

#endif