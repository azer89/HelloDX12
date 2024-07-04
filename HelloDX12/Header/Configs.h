#ifndef HELLO_DX12_CONFIGS
#define HELLO_DX12_CONFIGS

#include <string>

namespace AppConfig
{
	constexpr uint32_t InitialScreenWidth = 1200;
	constexpr uint32_t InitialScreenHeight = 900;

	// Number of frame in flight
	constexpr uint32_t FrameCount = 2;

	constexpr uint32_t MSAACount = 4;

	// TODO Set to white as a default
	constexpr float ClearColor[4]{ 0.0f, 0.1f, 0.4f, 1.0f };
	
	const std::string ScreenTitle = "Hello DX12";

	// Don't forget to change these absolute paths
	const std::string ShaderFolder = "C:/Users/azer/workspace/HelloDX12/HelloDX12/Shader/";
	const std::string ModelFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Models/";
	const std::string TextureFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Textures/";
	const std::string FontFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Fonts/";
};

namespace CameraConfig
{
	// These three below are in radian
	constexpr float Yaw = -1.5708f; // -90 degree
	constexpr float Pitch = 0.0f;
	constexpr float Zoom = 0.785398f; // 45 degree

	constexpr float Speed = 2.5f;
	constexpr float Sensitivity = 0.1f;

	// Reverse depth
	constexpr float Near = 100.0f;
	constexpr float Far = 0.1f;
}

#endif