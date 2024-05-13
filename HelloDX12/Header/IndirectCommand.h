#ifndef HELLO_DX12_INDIRECT_COMMAND
#define HELLO_DX12_INDIRECT_COMMAND

#include "d3dx12.h"

struct IndirectCommand
{
	D3D12_GPU_VIRTUAL_ADDRESS cbv;
	D3D12_DRAW_ARGUMENTS drawArguments;
};

#endif