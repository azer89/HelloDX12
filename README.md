
A rendering app written using C++ and DirectX 12.

<img width="700" alt="Sponza" src="https://github.com/azer89/HelloDX12/assets/790432/3eef4702-ca89-447f-afc9-0e2bd98b0cf9">

<img width="700" alt="Zaku" src="https://github.com/azer89/HelloDX12/assets/790432/3df855ca-0d11-41ac-8fb0-4883d8960f26">


### Features
* PBR
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
* Either use `HelloDX12/HelloDX12.sln` or run CMake
* Set absolute paths for assets and shaders in `HelloDX12/Header/Configs.h`
* That's it!

Dependencies: Agility SDK, [assimp](https://github.com/assimp/assimp), [D3D12MemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator), [DXC](https://github.com/microsoft/DirectXShaderCompiler), [glm](https://github.com/g-truc/glm), [imgui](https://github.com/ocornut/imgui), [stb](https://github.com/nothings/stb)
