
# Assimp
set(ASSIMP_NO_EXPORT ON CACHE BOOL "")
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "")
set(ASSIMP_INSTALL_PBD OFF CACHE BOOL "")
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "")
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "")
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "")
add_subdirectory(assimp)

# D3D12MA
add_subdirectory(D3D12MemoryAllocator)

# ImGui
add_library(imgui STATIC)
target_include_directories(imgui PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/imgui"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/backends"
)
target_sources(imgui PRIVATE 
    "${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.h"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_dx12.h"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_win32.h"

    "${CMAKE_CURRENT_LIST_DIR}/imgui/imgui.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_tables.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_dx12.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_win32.cpp"
)

# Group libraries into a folder
set_property(TARGET imgui PROPERTY FOLDER "External")
set_property(TARGET assimp PROPERTY FOLDER "External")
set_property(TARGET zlibstatic PROPERTY FOLDER "External")
set_property(TARGET D3D12MemoryAllocator PROPERTY FOLDER "External")

if(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs)
    set_property(TARGET UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTY FOLDER "External")
endif()