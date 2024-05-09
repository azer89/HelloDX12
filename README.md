
A rendering app written using C++ and DirectX 12. The shading is fairly basic as the project is still in its infancy.

<img width="600" alt="Zaku" src="https://github.com/azer89/HelloDX12/assets/790432/59e07438-9340-4527-92c6-097df28b6214">

### Features
* Blinn-Phong shading
* Tonemapping
* MSAA
* Vertex pulling
* Mipmap generator using compute shader
* Skybox from equirectangular HDR image
* Basic abstraction with simple resource state tracking and easier barrier insertion.

### Build
* Either use `HelloDX12/HelloDX12.sln` or run CMake
* Set absolute paths for assets and shaders in `HelloDX12/Header/Configs.h`
