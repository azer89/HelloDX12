#ifndef HELLO_DX12_INDIRECT_COMMAND
#define HELLO_DX12_INDIRECT_COMMAND

#include "d3dx12.h"

struct IndirectCommand
{
	uint32_t meshIndex{};
	D3D12_DRAW_ARGUMENTS drawArguments{};
};

#endif