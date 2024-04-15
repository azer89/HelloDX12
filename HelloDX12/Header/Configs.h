#ifndef HELLO_DX12_CONFIGS
#define HELLO_DX12_CONFIGS

#include <string>

namespace AppConfig
{
	constexpr uint32_t InitialScreenWidth = 800;
	constexpr uint32_t InitialScreenHeight = 600;

	// Number of frame in flight
	constexpr uint32_t FrameCount = 2;

	const std::string ScreenTitle = "Hello DX12";

	// Need to change these absolute paths
	const std::string ShaderFolder = "C:/Users/azer/workspace/HelloDX12/HelloDX12/Shader/";
	const std::string ModelFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Models/";
	const std::string TextureFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Textures/";
	const std::string FontFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Fonts/";
};

namespace CameraConfig
{
	constexpr float Yaw = -90.0f;
	constexpr float Pitch = 0.0f;
	constexpr float Speed = 2.5f;
	constexpr float Sensitivity = 0.1f;
	constexpr float Zoom = 45.0f;

	constexpr float Near = 0.1f;
	constexpr float Far = 100.0f;
}

#endif