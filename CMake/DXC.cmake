# Adapted from github.com/alipbcs/ZetaRay

function(SetupDXC DXC_DIR_RETURN)
    set(DXC_DIR "${PROJECT_SOURCE_DIR}/External/DXC")
    file(GLOB_RECURSE DXC_BIN_PATH "${DXC_DIR}/*dxc.exe")
    
    if(DXC_BIN_PATH STREQUAL "")
        set(URL "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2405/dxc_2024_05_24.zip")
        message(STATUS "Downloading DXC from ${URL}...")
        set(ARCHIVE_PATH "${DXC_DIR}/dxc.zip")
        file(DOWNLOAD "${URL}" "${ARCHIVE_PATH}" TIMEOUT 120)

        file(ARCHIVE_EXTRACT INPUT "${ARCHIVE_PATH}" DESTINATION "${DXC_DIR}")

        # Remove unused files
        file(REMOVE_RECURSE "${DXC_DIR}/bin/arm64")
        file(REMOVE_RECURSE "${DXC_DIR}/bin/x86")
        file(REMOVE_RECURSE "${DXC_DIR}/lib/arm64")
        file(REMOVE_RECURSE "${DXC_DIR}/lib/x86")
        file(REMOVE "${ARCHIVE_PATH}")

    endif()

    set(${DXC_DIR_RETURN} ${DXC_DIR} PARENT_SCOPE)
endfunction()