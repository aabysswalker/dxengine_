#if !defined(ASSETS_H)

#undef global
#undef local_global

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#define global static
#define local_global static

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <glm/glm.hpp>

#pragma pack(push, 1)
struct vertex
{
    glm::vec3 Pos;
    glm::vec2 Uv;
    glm::vec3 Normal;
};

struct texel_rgba8
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
    uint8_t Alpha;
};
#pragma pack(pop)

struct texture
{
    i32 Width;
    i32 Height;
    int* Texels;

    ID3D12Resource* GpuTexture;
    D3D12_GPU_DESCRIPTOR_HANDLE GpuDescriptor;
};

struct mesh
{
    int IndexOffset;
    int IndexCount;
    int VertexOffset;
    int VertexCount;
    int TextureId;
};

struct model
{
    int NumMeshes;
    mesh* MeshArray;

    int NumTextures;
    texture* TextureArray;
    
    int VertexCount;
    vertex* VertexArray;
    int IndexCount;
    int* IndexArray;

    ID3D12Resource* GpuVertexBuffer;
    ID3D12Resource* GpuIndexBuffer;
};

#define ASSETS_H
#endif
