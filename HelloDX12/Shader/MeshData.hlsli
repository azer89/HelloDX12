struct MeshData
{
    uint vertexOffset;
    uint indexOffset;

    uint modelMatrixIndex;

	// PBR Texture IDs
    uint albedo;
    uint normal;
    uint metalness;
    uint roughness;
    uint ao;
    uint emissive;
};