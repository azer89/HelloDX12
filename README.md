
A graphics renderer developed using C++ and DirectX 12.

<img width="700" alt="Sponza" src="https://github.com/azer89/HelloDX12/assets/790432/317aa44f-5771-4c99-a1bd-1f0da413a44a">

<img width="700" alt="Zaku" src="https://github.com/azer89/HelloDX12/assets/790432/18ff889b-5e18-4009-bc28-3c9d0585234a">

<img width="700" alt="Zaku" src="https://github.com/azer89/HelloDX12/assets/790432/3cb1c260-5c06-4ad5-a531-c69ecef3e6fa">

### Features
* PBR
* IBL
* Blinn-Phong
* Tonemapping
* MSAA
* Mipmap generator using compute shader
* Skybox from equirectangular HDR image
* Indirect draw
* Vertex pulling
* Bindless textures using unbounded array and dynamic indexing
* Basic abstraction with simple resource state tracking and easier barrier insertion

### Build
* Run CMake
* Set absolute paths for assets and shaders in `HelloDX12/Header/Configs.h`
* That's it!

Dependencies: Agility SDK, [assimp](https://github.com/assimp/assimp), [D3D12MemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator), [DXC](https://github.com/microsoft/DirectXShaderCompiler), [glm](https://github.com/g-truc/glm), [imgui](https://github.com/ocornut/imgui), [stb](https://github.com/nothings/stb)
