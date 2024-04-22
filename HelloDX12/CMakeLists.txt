include("${CMAKE_INCLUDE_DIR}/AgilitySDK.cmake")
include("${CMAKE_INCLUDE_DIR}/Copy.cmake")

file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/Shader/*")
file(GLOB_RECURSE H_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/Header/*.h")
file(GLOB_RECURSE CPP_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/Source/*.cpp")

# Do not compile HLSL files
file(GLOB_RECURSE HLSL_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/Shader/*.hlsl")
set_source_files_properties(SOURCE ${HLSL_FILES} PROPERTIES VS_SETTINGS "ExcludedFromBuild=true")

# Set WinMain as the entry point
set(CMAKE_EXE_LINKER_FLAGS "/SUBSYSTEM:WINDOWS")

# Set unicode
add_definitions(-DUNICODE -D_UNICODE)

add_executable(HelloDX12 
    ${H_FILES}
    ${CPP_FILES}
    ${SHADER_FILES}
)

source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/Shader" PREFIX "Shaders" FILES ${SHADER_FILES})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/Header" PREFIX "Header Files" FILES ${H_FILES})
source_group(TREE "${CMAKE_CURRENT_LIST_DIR}/Source" PREFIX "Source Files" FILES ${CPP_FILES})

target_compile_definitions(HelloDX12 PRIVATE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>

    WIN32
    _WINDOWS
    _CRT_SECURE_NO_WARNINGS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

# Tracy does not like /ZI so set it to /Zi
target_compile_options(HelloDX12 PRIVATE "/Zi")

# Include directories
set(HEADER_FOLDER "${CMAKE_CURRENT_LIST_DIR}/Header")
target_include_directories(HelloDX12 PRIVATE "${HEADER_FOLDER}")
file(GLOB ITEMS LIST_DIRECTORIES true "${HEADER_FOLDER}/*")
foreach(item ${ITEMS})
    if(IS_DIRECTORY ${item})
        target_include_directories(HelloDX12 PRIVATE ${item})
    endif()
endforeach()

# Agility SDK
SetupAgilitySDK(AGILITY_SDK_VER)
add_library(DX12AgilitySDK INTERFACE)
set(DX12AgilitySDK_BIN
    "${PROJECT_SOURCE_DIR}/External/D3D12/${AGILITY_SDK_VER}/D3D12Core.dll"
    "${PROJECT_SOURCE_DIR}/External/D3D12/${AGILITY_SDK_VER}/d3d12SDKLayers.dll")

Copy("${DX12AgilitySDK_BIN}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12/" CopyDX12AgilitySDKBins)
add_dependencies(DX12AgilitySDK CopyDX12AgilitySDKBins)

# Include directories from dependencies
target_include_directories(HelloDX12 PRIVATE
    "${PROJECT_SOURCE_DIR}/External/stb"
    "${PROJECT_SOURCE_DIR}/External/glm"
    "${PROJECT_SOURCE_DIR}/External/D3D12/${AGILITY_SDK_VER}/d3dx12"
    "${PROJECT_SOURCE_DIR}/External/assimp/include"
    "${PROJECT_SOURCE_DIR}/External/Binaries/assimp/include"
    "${PROJECT_SOURCE_DIR}/External/D3D12MemoryAllocator/include"
)

# Set dependencies
target_link_libraries(HelloDX12 PRIVATE assimp D3D12MemoryAllocator)
target_link_libraries(HelloDX12 PRIVATE d3d12.lib dxgi.lib d3dcompiler.lib dxguid.lib) 