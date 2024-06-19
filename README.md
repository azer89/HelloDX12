
A graphics renderer developed using C++ and DirectX 12.

<img width="700" alt="" src="https://github.com/azer89/HelloDX12/assets/790432/6caea4f2-64c6-4c9f-baea-9e59d53aba0e">

<img width="700" alt="" src="https://github.com/azer89/HelloDX12/assets/790432/4bb91ea5-a8a2-4ab1-a2d1-448a2b3a9064">

<img width="700" alt="" src="https://github.com/azer89/HelloDX12/assets/790432/023268cf-dfad-46b3-9a1b-74df7d20578e">

### Features
* PBR
* IBL
* Indirect draw
* Vertex pulling
* Bindless textures using unbounded array and dynamic indexing
* Basic abstraction with simple resource state tracking and easier barrier insertion
* Reverse depth
* Blinn-Phong
* Tonemapping
* MSAA
* Mipmap generator using compute shader
* Skybox from equirectangular HDR image

### Build
* Run CMake
* Set absolute paths for assets and shaders in `HelloDX12/Header/Configs.h`
* That's it!

Dependencies: Agility SDK, [assimp](https://github.com/assimp/assimp), [D3D12MemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator), [DXC](https://github.com/microsoft/DirectXShaderCompiler), [glm](https://github.com/g-truc/glm), [imgui](https://github.com/ocornut/imgui), [stb](https://github.com/nothings/stb)
