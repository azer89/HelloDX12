include("${CMAKE_INCLUDE_DIR}/AgilitySDK.cmake")
include("${CMAKE_INCLUDE_DIR}/DXC.cmake")

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
SetupAgilitySDK(AGILITY_DIRECTORY)

# DXC
SetupDXC(DXC_DIRECTORY)

set(DXC_DLL_FILES 
    dxcompiler.dll
    dxil.dll)

set(CONFIG_TYPES
    "Debug"
    "Release")

# TODO Currently copy DXC dlls to both Debug and Release folders
foreach(DLL_FILE ${DXC_DLL_FILES})
    foreach(CONFIG_TYPE ${CONFIG_TYPES})
        add_custom_command(
            TARGET HelloDX12 POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy
            "${DXC_DIRECTORY}/bin/x64/${DLL_FILE}"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CONFIG_TYPE}/${DLL_FILE}")
    endforeach()
endforeach()

# Include directories from dependencies
target_include_directories(HelloDX12 PRIVATE
    "${DXC_DIRECTORY}/inc"
    "${AGILITY_DIRECTORY}/d3dx12"
    "${PROJECT_SOURCE_DIR}/External/stb"
    "${PROJECT_SOURCE_DIR}/External/glm"
    "${PROJECT_SOURCE_DIR}/External/imgui"
    "${PROJECT_SOURCE_DIR}/External/imgui/backends"
    "${PROJECT_SOURCE_DIR}/External/assimp/include"
    "${PROJECT_SOURCE_DIR}/External/Binaries/assimp/include"
    "${PROJECT_SOURCE_DIR}/External/D3D12MemoryAllocator/include"
)

# Set dependencies
target_link_libraries(HelloDX12 PRIVATE assimp imgui D3D12MemoryAllocator)
target_link_libraries(HelloDX12 PRIVATE d3d12.lib dxgi.lib d3dcompiler.lib dxguid.lib)
target_link_libraries(HelloDX12 PRIVATE "${DXC_DIRECTORY}/lib/x64/dxcompiler.lib")