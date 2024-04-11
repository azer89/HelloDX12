#ifndef HELLO_DX12_CONFIGS
#define HELLO_DX12_CONFIGS

#include <string>

namespace AppConfig
{
	constexpr uint32_t InitialScreenWidth = 1600;
	constexpr uint32_t InitialScreenHeight = 1200;

	// Number of frame in flight
	constexpr uint32_t FrameCount = 2;

	const std::string ScreenTitle = "Hello DX12";

	// Need to change these absolute paths
	const std::string ShaderFolder = "C:/Users/azer/workspace/HelloDX12/HelloDX12/Shaders/";
	const std::string ModelFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Models/";
	const std::string TextureFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Textures/";
	const std::string FontFolder = "C:/Users/azer/workspace/HelloDX12/Assets/Fonts/";
};

#endif