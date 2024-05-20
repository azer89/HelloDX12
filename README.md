
A rendering app written using C++ and DirectX 12.

<img width="600" alt="Zaku" src="https://github.com/azer89/HelloDX12/assets/790432/3f08d9fa-c479-44fc-a373-8d14a6ffff27">

<img width="600" alt="Sponza" src="https://github.com/azer89/HelloDX12/assets/790432/1516325f-11e4-49d1-a159-a4c615fd3fbd">

### Features
* PBR.
* Blinn-Phong.
* Tonemapping.
* MSAA.
* Mipmap generator using compute shader.
* Skybox from equirectangular HDR image.
* Indirect draw.
* Vertex pulling.
* Bindless textures using unbounded array and dynamic indexing.
* Basic abstraction with simple resource state tracking and easier barrier insertion.

### Build
* Either use `HelloDX12/HelloDX12.sln` or run CMake.
* Set absolute paths for assets and shaders in `HelloDX12/Header/Configs.h`.
* That's it!

Dependencies:
* Agility SDK
* assimp
* D3D12MemoryAllocator
* DirectXShaderCompiler (DXC)
* glm
* stb
