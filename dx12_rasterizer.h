#if !defined(DX12_RASTERIZER_H)

#include <d3d12.h>
#include <dxgi1_3.h>

struct dir_light_buffer_cpu
{
    glm::vec3 LightColor;
    float LightAmbientIntensity;
    glm::vec3 LightDirection;
    int NumPointLights;
    glm::vec3 CameraPos;
};

struct point_light_cpu
{
    glm::vec3 Pos;
    float DivisorConstant;
    glm::vec3 Color;
};

struct transform_buffer_cpu
{
    m4 WVPTransform;
    m4 WTransform;
    m4 NormalWTransform;
    float Shininess;
    float SpecularStrength;
};

struct dx12_arena
{
    uint64_t Size;
    uint64_t Used;
    ID3D12Heap* Heap;
};

struct dx12_upload_arena
{
    uint64_t Size;
    uint64_t Used;
    ID3D12Resource* GpuBuffer;
    uint8_t* CpuPtr;
};

struct dx12_descriptor_heap
{
    ID3D12DescriptorHeap* Heap;
    uint64_t StepSize;
    uint64_t MaxNumElements;
    uint64_t CurrElement;
};

struct dx12_rasterizer
{
    IDXGIAdapter1* Adapter;
    ID3D12Device* Device;
    ID3D12CommandQueue* CommandQueue;

    dx12_upload_arena UploadArena;
    dx12_arena RtvArena;
    dx12_arena BufferArena;
    dx12_arena TextureArena;

    int RenderWidth;
    int RenderHeight;
    IDXGISwapChain1* SwapChain;
    int CurrentFrameIndex;
    ID3D12Resource* FrameBuffers[2];
    D3D12_CPU_DESCRIPTOR_HANDLE FrameBufferDescriptors[2];

    D3D12_CPU_DESCRIPTOR_HANDLE DepthDescriptor;
    ID3D12Resource* DepthBuffer;
    
    ID3D12CommandAllocator* CommandAllocator;

    ID3D12GraphicsCommandList* CommandList;
    ID3D12Fence* Fence;
    UINT64 FenceValue;

    dx12_descriptor_heap RtvHeap;
    dx12_descriptor_heap DsvHeap;
    dx12_descriptor_heap ShaderDescHeap;

    ID3D12Resource* SponzaTransformBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE SponzaTransformDescriptor;

    // NOTE: Дані пов'язані з світлом
#define NUM_POINT_LIGHTS 2
    ID3D12Resource* PlTransformBuffers[NUM_POINT_LIGHTS];
    D3D12_GPU_DESCRIPTOR_HANDLE PlTransformDescriptors[NUM_POINT_LIGHTS];
    ID3D12Resource* DirLightBuffer;
    ID3D12Resource* PointLightBuffer;
    D3D12_GPU_DESCRIPTOR_HANDLE LightDescriptor;
    
    // NOTE: Графічний конвеєр
    ID3D12RootSignature* ModelRootSignature;
    ID3D12PipelineState* ModelPipeline;
};

#define DX12_RASTERIZER_H
#endif
