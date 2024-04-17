#ifndef HELLO_DX12_SHAPE_GENERATOR
#define HELLO_DX12_SHAPE_GENERATOR

#include "VertexData.h"
#include <vector>

namespace ShapeGenerator
{
	void Cube(std::vector<VertexData>& vertices, std::vector<uint32_t>& indices);
}

#endif