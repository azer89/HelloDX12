# Adapted from github.com/alipbcs/ZetaRay

function(SetupAgilitySDK SDK_DIR_RETURN)
    set(VER "1.613.2")
    set(SDK_DIR "${PROJECT_SOURCE_DIR}/External/D3D12/${VER}")

    file(GLOB_RECURSE CORE_DLL_PATH "${SDK_DIR}/*D3D12Core.dll")
    
    if(CORE_DLL_PATH STREQUAL "")
        file(MAKE_DIRECTORY ${SDK_DIR})

        # Download from nuget
        set(URL "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/${VER}")
        message(STATUS "Downloading Agility SDK from ${URL}...")
        set(ARCHIVE_PATH "${SDK_DIR}/temp/agility.zip")
        file(DOWNLOAD "${URL}" "${ARCHIVE_PATH}" TIMEOUT 120)       
        file(ARCHIVE_EXTRACT INPUT "${ARCHIVE_PATH}" DESTINATION "${SDK_DIR}/temp")
        
        # Copy d3dx12
        file(GLOB_RECURSE D3DX12_FILES "${SDK_DIR}/temp/build/native/include/*.h")
        file(COPY ${D3DX12_FILES} DESTINATION ${SDK_DIR}/d3dx12)

        # Copy headers
        file(GLOB_RECURSE SDK_HEADERS "${SDK_DIR}/temp/build/native/include/*.h")
        file(COPY ${SDK_HEADERS} DESTINATION ${SDK_DIR})
        
        if(SDK_HEADERS STREQUAL "")
            message(FATAL_ERROR "Setting up Agility SDK failed.")
        endif()

        # Copy binaries
        set(DLLS
            "${SDK_DIR}/temp/build/native/bin/x64/D3D12Core.dll"
            "${SDK_DIR}/temp/build/native/bin/x64/d3d12SDKLayers.dll")
        
        file(COPY ${DLLS} DESTINATION ${SDK_DIR})

        # Cleanup
        file(REMOVE_RECURSE "${SDK_DIR}/temp")
    endif()

    set(${SDK_DIR_RETURN} ${SDK_DIR} PARENT_SCOPE)
endfunction()