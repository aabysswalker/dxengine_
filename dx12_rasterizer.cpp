
void ThrowIfFailed(HRESULT Result)
{
    if (Result != S_OK)
    {
        InvalidCodePath;
    }
}

int Dx12GetBytesPerPixel(DXGI_FORMAT Format)
{
    int Result = 0;
    
    switch (Format)
    {
        case DXGI_FORMAT_R32G32B32A32_TYPELESS:
        case DXGI_FORMAT_R32G32B32A32_FLOAT:
        case DXGI_FORMAT_R32G32B32A32_UINT:
        case DXGI_FORMAT_R32G32B32A32_SINT:
        {
            Result = 16;
        } break;

        case DXGI_FORMAT_R16G16B16A16_TYPELESS:
        case DXGI_FORMAT_R16G16B16A16_FLOAT:
        case DXGI_FORMAT_R16G16B16A16_UNORM:
        case DXGI_FORMAT_R16G16B16A16_UINT:
        case DXGI_FORMAT_R16G16B16A16_SNORM:
        case DXGI_FORMAT_R16G16B16A16_SINT:
        case DXGI_FORMAT_R32G32_TYPELESS:
        case DXGI_FORMAT_R32G32_FLOAT:
        case DXGI_FORMAT_R32G32_UINT:
        case DXGI_FORMAT_R32G32_SINT:
        {
            Result = 8;
        } break;
        
        case DXGI_FORMAT_R8G8B8A8_TYPELESS:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UINT:
        case DXGI_FORMAT_R8G8B8A8_SNORM:
        case DXGI_FORMAT_R8G8B8A8_SINT:
        case DXGI_FORMAT_R16G16_TYPELESS:
        case DXGI_FORMAT_R16G16_FLOAT:
        case DXGI_FORMAT_R16G16_UNORM:
        case DXGI_FORMAT_R16G16_UINT:
        case DXGI_FORMAT_R16G16_SNORM:
        case DXGI_FORMAT_R16G16_SINT:
        case DXGI_FORMAT_R32_TYPELESS:
        case DXGI_FORMAT_D32_FLOAT:
        case DXGI_FORMAT_R32_FLOAT:
        case DXGI_FORMAT_R32_UINT:
        case DXGI_FORMAT_R32_SINT:
        {
            Result = 4;
        } break;

        case DXGI_FORMAT_R8G8_TYPELESS:
        case DXGI_FORMAT_R8G8_UNORM:
        case DXGI_FORMAT_R8G8_UINT:
        case DXGI_FORMAT_R8G8_SNORM:
        case DXGI_FORMAT_R8G8_SINT:
        case DXGI_FORMAT_R16_TYPELESS:
        case DXGI_FORMAT_R16_FLOAT:
        case DXGI_FORMAT_D16_UNORM:
        case DXGI_FORMAT_R16_UNORM:
        case DXGI_FORMAT_R16_UINT:
        case DXGI_FORMAT_R16_SNORM:
        case DXGI_FORMAT_R16_SINT:
        {
            Result = 2;
        } break;

        case DXGI_FORMAT_R8_TYPELESS:
        case DXGI_FORMAT_R8_UNORM:
        case DXGI_FORMAT_R8_UINT:
        case DXGI_FORMAT_R8_SNORM:
        case DXGI_FORMAT_R8_SINT:
        case DXGI_FORMAT_A8_UNORM:
        {
            Result = 1;
        } break;
    }

    return Result;
}

//
// NOTE: Дескрипторна Купа
//

dx12_descriptor_heap Dx12DescriptorHeapCreate(dx12_rasterizer* Rasterizer,
                                              D3D12_DESCRIPTOR_HEAP_TYPE Type,
                                              uint64_t NumDescriptors,
                                              D3D12_DESCRIPTOR_HEAP_FLAGS Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE)
{
    dx12_descriptor_heap Result = {};
    ID3D12Device* Device = Rasterizer->Device;

    Result.MaxNumElements = NumDescriptors;
    Result.StepSize = Device->GetDescriptorHandleIncrementSize(Type);

    D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
    Desc.Type = Type;
    Desc.NumDescriptors = NumDescriptors;
    Desc.Flags = Flags;
    ThrowIfFailed(Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Result.Heap)));
    
    return Result;
}

void Dx12DescriptorAllocate(dx12_descriptor_heap* Heap,
                            D3D12_CPU_DESCRIPTOR_HANDLE* OutCpuHandle,
                            D3D12_GPU_DESCRIPTOR_HANDLE* OutGpuHandle)
{
    Assert(Heap->CurrElement < Heap->MaxNumElements);
    
    if (OutCpuHandle)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE CpuHandle = Heap->Heap->GetCPUDescriptorHandleForHeapStart();
        CpuHandle.ptr += Heap->StepSize * Heap->CurrElement;
        *OutCpuHandle = CpuHandle;
    }

    if (OutGpuHandle)
    {
        D3D12_GPU_DESCRIPTOR_HANDLE GpuHandle = Heap->Heap->GetGPUDescriptorHandleForHeapStart();
        GpuHandle.ptr += Heap->StepSize * Heap->CurrElement;
        *OutGpuHandle = GpuHandle;
    }

    Heap->CurrElement += 1;
}

//
// NOTE: Лінійна Арена
//

dx12_arena Dx12ArenaCreate(ID3D12Device* Device, D3D12_HEAP_TYPE Type, uint64_t Size,
                           D3D12_HEAP_FLAGS Flags)
{
    dx12_arena Result = {};
    Result.Size = Size;

    D3D12_HEAP_DESC Desc = {};
    Desc.SizeInBytes = Size;
    Desc.Properties.Type = Type;
    Desc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    Desc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    Desc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
    Desc.Flags = Flags;

    ThrowIfFailed(Device->CreateHeap(&Desc, IID_PPV_ARGS(&Result.Heap)));
    
    return Result;
}

ID3D12Resource* Dx12CreateResource(dx12_rasterizer* Rasterizer, dx12_arena* Arena,
                                   D3D12_RESOURCE_DESC* Desc,
                                   D3D12_RESOURCE_STATES InitialState,
                                   D3D12_CLEAR_VALUE* ClearValues)
{
    ID3D12Resource* Result = 0;
    ID3D12Device* Device = Rasterizer->Device;

    D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Device->GetResourceAllocationInfo(0, 1, Desc);
    uint64_t GpuAlignedOffset = Align(Arena->Used, AllocationInfo.Alignment);
    Assert((GpuAlignedOffset + AllocationInfo.SizeInBytes) < Arena->Size);
    
    ThrowIfFailed(Device->CreatePlacedResource(Arena->Heap, GpuAlignedOffset,
                                               Desc, InitialState, ClearValues,
                                               IID_PPV_ARGS(&Result)));
    Arena->Used = GpuAlignedOffset + AllocationInfo.SizeInBytes;

    return Result;
}

//
// NOTE: Арена Завантаження
//

dx12_upload_arena Dx12UploadArenaCreate(dx12_rasterizer* Rasterizer, uint64_t Size)
{
    dx12_upload_arena Result = {};
    Result.Size = Size;

    ID3D12Device* Device = Rasterizer->Device;
            
    D3D12_HEAP_PROPERTIES HeapProperties = {};
    HeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
    HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Size;
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    ThrowIfFailed(Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE, &Desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                                  0, IID_PPV_ARGS(&Result.GpuBuffer)));
    Result.GpuBuffer->Map(0, 0, (void**)&Result.CpuPtr);

    return Result;
}

uint8_t* Dx12UploadArenaPushSize(dx12_upload_arena* Arena, uint64_t Size, uint64_t* OutOffset)
{
    uint64_t AlignedOffset = Align(Arena->Used, D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT);
    Assert((AlignedOffset + Size) < Arena->Size);
    
    uint8_t* Result = Arena->CpuPtr + AlignedOffset;
    Arena->Used = AlignedOffset + Size;
    *OutOffset = AlignedOffset;
    
    return Result;
}

void Dx12ClearUploadArena(dx12_upload_arena* Arena)
{
    Arena->Used = 0;
}

//
// NOTE: Творення Ресурсів зі копiювання
//

void Dx12CopyDataToBuffer(dx12_rasterizer* Rasterizer,
                          D3D12_RESOURCE_STATES StartState,
                          D3D12_RESOURCE_STATES EndState,
                          void* Data,
                          uint64_t DataSize,
                          ID3D12Resource* GpuBuffer)
{
    ID3D12Device* Device = Rasterizer->Device;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    uint64_t UploadOffset = 0;
    {
        uint8_t* Dest = Dx12UploadArenaPushSize(&Rasterizer->UploadArena, DataSize, &UploadOffset);
        memcpy(Dest, Data, DataSize);
    }

    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = GpuBuffer;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = StartState;
        Barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        CommandList->ResourceBarrier(1, &Barrier);
    }

    CommandList->CopyBufferRegion(GpuBuffer, 0, Rasterizer->UploadArena.GpuBuffer, UploadOffset, DataSize);

    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = GpuBuffer;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        Barrier.Transition.StateAfter = EndState;
        CommandList->ResourceBarrier(1, &Barrier);
    }
}

ID3D12Resource* Dx12CreateBufferAsset(dx12_rasterizer* Rasterizer,
                                      D3D12_RESOURCE_DESC* Desc,
                                      D3D12_RESOURCE_STATES InitialState,
                                      void* BufferData)
{
    ID3D12Resource* Result = 0;

    ID3D12Device* Device = Rasterizer->Device;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    uint64_t UploadOffset = 0;
    {
        uint8_t* Dest = Dx12UploadArenaPushSize(&Rasterizer->UploadArena,
                                           Desc->Width, &UploadOffset);
        memcpy(Dest, BufferData, Desc->Width);
    }

    Result = Dx12CreateResource(Rasterizer, &Rasterizer->BufferArena, Desc,
                                D3D12_RESOURCE_STATE_COPY_DEST, 0);
    CommandList->CopyBufferRegion(Result, 0,
                                  Rasterizer->UploadArena.GpuBuffer,
                                  UploadOffset, Desc->Width);

    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = Result;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        Barrier.Transition.StateAfter = InitialState;
        CommandList->ResourceBarrier(1, &Barrier);
    }
    
    return Result;
}

ID3D12Resource* Dx12CreateBufferAsset(dx12_rasterizer* Rasterizer, int Size, D3D12_RESOURCE_STATES State, void* Data)
{
    ID3D12Resource* Result = 0;
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Size;
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    Result = Dx12CreateBufferAsset(Rasterizer, &Desc, State, Data);

    return Result;
}

void Dx12CreateConstantBuffer(dx12_rasterizer* Rasterizer, int Size, ID3D12Resource** OutResource,
                              D3D12_GPU_DESCRIPTOR_HANDLE* OutDescriptor)
{
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    Desc.Width = Align(Size, 256);
    Desc.Height = 1;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = 1;
    Desc.Format = DXGI_FORMAT_UNKNOWN;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    *OutResource = Dx12CreateResource(Rasterizer,
                                      &Rasterizer->BufferArena,
                                      &Desc,
                                      D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                                      0);

    D3D12_CONSTANT_BUFFER_VIEW_DESC CbvDesc = {};
    CbvDesc.BufferLocation = OutResource[0]->GetGPUVirtualAddress();
    CbvDesc.SizeInBytes = Desc.Width;

    D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
    Dx12DescriptorAllocate(&Rasterizer->ShaderDescHeap, &CpuDescriptor, OutDescriptor);
    Rasterizer->Device->CreateConstantBufferView(&CbvDesc, CpuDescriptor);
}

ID3D12Resource* Dx12CreateTextureAsset(dx12_rasterizer* Rasterizer,
                                       D3D12_RESOURCE_DESC* Desc,
                                       D3D12_RESOURCE_STATES InitialState,
                                       void* Texels)
{
#define MAX_MIP_LEVELS 32
    ID3D12Resource* Result = 0;

    ID3D12Device* Device = Rasterizer->Device;
    ID3D12GraphicsCommandList* CommandList = Rasterizer->CommandList;

    // NOTE: Ініцюємо дані для завантаження мип-рівенів
    D3D12_RESOURCE_ALLOCATION_INFO AllocationInfo = Device->GetResourceAllocationInfo(0, 1, Desc);
    uint64_t UploadSize = 0;
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT MipFootPrints[MAX_MIP_LEVELS] = {};
    Device->GetCopyableFootprints(Desc, 0, Desc->MipLevels, 0, MipFootPrints, 0, 0, &UploadSize);

    uint64_t UploadOffset = 0;
    uint8_t* UploadTexels = Dx12UploadArenaPushSize(&Rasterizer->UploadArena, UploadSize, &UploadOffset);
    
    uint64_t BytesPerPixel = Dx12GetBytesPerPixel(Desc->Format);

    texel_rgba8* MipMemory = Desc->MipLevels > 1 ? (texel_rgba8*)malloc(UploadSize) : 0;

    {
        D3D12_PLACED_SUBRESOURCE_FOOTPRINT* CurrFootPrint = MipFootPrints + 0;
        
        CurrFootPrint->Offset += UploadOffset;
        for (int Y = 0; Y < Desc->Height; ++Y)
        {
            uint8_t* Src = (uint8_t*)Texels + (Y * CurrFootPrint->Footprint.Width) * BytesPerPixel;
            uint8_t* Dest = UploadTexels + (Y * CurrFootPrint->Footprint.RowPitch);
            memcpy(Dest, Src, BytesPerPixel * CurrFootPrint->Footprint.Width);
        }
    }

    {
        texel_rgba8* SrcMipStart = MipMemory - MipFootPrints[0].Footprint.Width * MipFootPrints[0].Footprint.Height;
        texel_rgba8* DstMipStart = MipMemory;
        for (int MipId = 1; MipId < Desc->MipLevels; ++MipId)
        {
            Assert(Desc->Format == DXGI_FORMAT_R8G8B8A8_UNORM);
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT* PrevFootPrint = MipFootPrints + MipId - 1;
            D3D12_PLACED_SUBRESOURCE_FOOTPRINT* CurrFootPrint = MipFootPrints + MipId;

            texel_rgba8* SrcTexelBase = MipId == 1 ? (texel_rgba8*)Texels : SrcMipStart;
            for (int Y = 0; Y < CurrFootPrint->Footprint.Height; ++Y)
            {
                for (int X = 0; X < CurrFootPrint->Footprint.Width; ++X)
                {
                    texel_rgba8* DstTexel = DstMipStart + Y * CurrFootPrint->Footprint.Width + X;
                    
                    texel_rgba8* SrcTexel00 = SrcTexelBase + (2*Y + 0) * PrevFootPrint->Footprint.Width + 2*X + 0;
                    texel_rgba8* SrcTexel01 = SrcTexelBase + (2*Y + 0) * PrevFootPrint->Footprint.Width + 2*X + 1;
                    texel_rgba8* SrcTexel10 = SrcTexelBase + (2*Y + 1) * PrevFootPrint->Footprint.Width + 2*X + 0;
                    texel_rgba8* SrcTexel11 = SrcTexelBase + (2*Y + 1) * PrevFootPrint->Footprint.Width + 2*X + 1;

                    DstTexel->Red = uint8_t(round(float(SrcTexel00->Red + SrcTexel01->Red + SrcTexel10->Red + SrcTexel11->Red) / 4.0f));
                    DstTexel->Green = uint8_t(round(float(SrcTexel00->Green + SrcTexel01->Green + SrcTexel10->Green + SrcTexel11->Green) / 4.0f));
                    DstTexel->Blue = uint8_t(round(float(SrcTexel00->Blue + SrcTexel01->Blue + SrcTexel10->Blue + SrcTexel11->Blue) / 4.0f));
                    DstTexel->Alpha = uint8_t(round(float(SrcTexel00->Alpha + SrcTexel01->Alpha + SrcTexel10->Alpha + SrcTexel11->Alpha) / 4.0f));
                }
            }

            {
                texel_rgba8* SrcRowY = DstMipStart;
                uint8_t* DstRowY = UploadTexels + CurrFootPrint->Offset;
                CurrFootPrint->Offset += UploadOffset;
                
                for (int Y = 0; Y < CurrFootPrint->Footprint.Height; ++Y)
                {
                    memcpy(DstRowY, SrcRowY, BytesPerPixel * CurrFootPrint->Footprint.Width);
                    
                    DstRowY += CurrFootPrint->Footprint.RowPitch;
                    SrcRowY += CurrFootPrint->Footprint.Width;
                }
            }

            SrcMipStart += PrevFootPrint->Footprint.Width * PrevFootPrint->Footprint.Height;
            DstMipStart += CurrFootPrint->Footprint.Width * CurrFootPrint->Footprint.Height;
        }
    }
    
    if (MipMemory)
    {
        free(MipMemory);
    }

    Result = Dx12CreateResource(Rasterizer, &Rasterizer->TextureArena,
                                Desc, D3D12_RESOURCE_STATE_COPY_DEST, 0);

    for (int MipId = 0; MipId < Desc->MipLevels; ++MipId)
    {
        D3D12_TEXTURE_COPY_LOCATION SrcRegion = {};
        SrcRegion.pResource = Rasterizer->UploadArena.GpuBuffer;
        SrcRegion.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
        SrcRegion.PlacedFootprint = MipFootPrints[MipId];

        D3D12_TEXTURE_COPY_LOCATION DstRegion = {};
        DstRegion.pResource = Result;
        DstRegion.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
        DstRegion.SubresourceIndex = MipId;

        CommandList->CopyTextureRegion(&DstRegion, 0, 0, 0, &SrcRegion, nullptr);
    }

    {
        D3D12_RESOURCE_BARRIER Barrier = {};
        Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        Barrier.Transition.pResource = Result;
        Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        Barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        Barrier.Transition.StateAfter = InitialState;
        CommandList->ResourceBarrier(1, &Barrier);
    }
    
    return Result;
}

void Dx12CreateTexture(dx12_rasterizer* Rasterizer, int Width, int Height, uint8_t* Texels,
                       ID3D12Resource** OutResource, D3D12_GPU_DESCRIPTOR_HANDLE* OutDescriptor)
{
    D3D12_RESOURCE_DESC Desc = {};
    Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    Desc.Width = Width;
    Desc.Height = Height;
    Desc.DepthOrArraySize = 1;
    Desc.MipLevels = int(ceil(log2(std::max(uint64_t(Desc.Width), uint64_t(Desc.Height)))) + 1);
    Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Desc.SampleDesc.Count = 1;
    Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    *OutResource = Dx12CreateTextureAsset(Rasterizer,
                                          &Desc,
                                          D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                          Texels);

    D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.Format = Desc.Format;
    SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    SrvDesc.Shader4ComponentMapping = D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(0, 1, 2, 3);
    SrvDesc.Texture2D.MostDetailedMip = 0;
    SrvDesc.Texture2D.MipLevels = Desc.MipLevels;
    SrvDesc.Texture2D.PlaneSlice = 0;

    D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
    Dx12DescriptorAllocate(&Rasterizer->ShaderDescHeap, &CpuDescriptor, OutDescriptor);
    Rasterizer->Device->CreateShaderResourceView(*OutResource, &SrvDesc, CpuDescriptor);
}

void Dx12UploadTransformBuffer(dx12_rasterizer* Rasterizer,
                               ID3D12Resource* Resource,
                               m4 WTransform,
                               m4 VPTransform,
                               float Shininess,
                               float SpecularStrength)
{
    transform_buffer_cpu TransformBufferCopy = {};
    TransformBufferCopy.WTransform = WTransform;
    TransformBufferCopy.WVPTransform = VPTransform * WTransform;
    TransformBufferCopy.NormalWTransform = Transpose(Inverse(WTransform));
    TransformBufferCopy.Shininess = Shininess;
    TransformBufferCopy.SpecularStrength = SpecularStrength;
    Dx12CopyDataToBuffer(Rasterizer,
                         D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                         D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
                         &TransformBufferCopy,
                         sizeof(TransformBufferCopy),
                         Resource);
}

void Dx12RenderModel(ID3D12GraphicsCommandList* CommandList, model* Model, D3D12_GPU_DESCRIPTOR_HANDLE TransformDescriptor)
{
    CommandList->SetGraphicsRootDescriptorTable(2, TransformDescriptor);

    {
        D3D12_INDEX_BUFFER_VIEW View = {};
        View.BufferLocation = Model->GpuIndexBuffer->GetGPUVirtualAddress();
        View.SizeInBytes = sizeof(int) * Model->IndexCount;
        View.Format = DXGI_FORMAT_R32_UINT;
        CommandList->IASetIndexBuffer(&View);
    }

    {
        D3D12_VERTEX_BUFFER_VIEW Views[1] = {};
        Views[0].BufferLocation = Model->GpuVertexBuffer->GetGPUVirtualAddress();
        Views[0].SizeInBytes = sizeof(vertex) * Model->VertexCount;
        Views[0].StrideInBytes = sizeof(vertex);
        CommandList->IASetVertexBuffers(0, 1, Views);
    }

    for (int MeshId = 0; MeshId < Model->NumMeshes; ++MeshId)
    {
        mesh* CurrMesh = Model->MeshArray + MeshId;
        texture* CurrTexture = Model->TextureArray + CurrMesh->TextureId;
                        
        CommandList->SetGraphicsRootDescriptorTable(0, CurrTexture->GpuDescriptor);
        CommandList->DrawIndexedInstanced(CurrMesh->IndexCount, 1, CurrMesh->IndexOffset, CurrMesh->VertexOffset, 0);
    }
}

D3D12_SHADER_BYTECODE Dx12LoadShader(const char* FileName)
{
    D3D12_SHADER_BYTECODE Result = {};

    FILE* File;
    errno_t err = fopen_s(&File, FileName, "rb");
    Assert(err == 0 && File != NULL);

    fseek(File, 0, SEEK_END);
    Result.BytecodeLength = ftell(File);
    fseek(File, 0, SEEK_SET);

    void* FileData = malloc(Result.BytecodeLength);
    fread(FileData, Result.BytecodeLength, 1, File);
    Result.pShaderBytecode = FileData;

    fclose(File);

    return Result;
}


dx12_rasterizer Dx12RasterizerCreate(HWND WindowHandle, int Width, int Height)
{
    dx12_rasterizer Result = {};
    Result.RenderWidth = Width;
    Result.RenderHeight = Height;

    IDXGIFactory2* Factory = 0;
    ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&Factory)));
 
    ID3D12Debug1* Debug;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&Debug)));
    Debug->EnableDebugLayer();
    Debug->SetEnableGPUBasedValidation(true);
    
    for (int AdapterIndex = 0;
         Factory->EnumAdapters1(AdapterIndex, &Result.Adapter) != DXGI_ERROR_NOT_FOUND;
         ++AdapterIndex)
    {
        DXGI_ADAPTER_DESC1 Desc;
        Result.Adapter->GetDesc1(&Desc);

        if (Desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
        {
            continue;
        }

        if (SUCCEEDED(D3D12CreateDevice(Result.Adapter, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&Result.Device))))
        {
            break;
        }
    }

    ID3D12Device* Device = Result.Device;
    
    {
        D3D12_COMMAND_QUEUE_DESC Desc = {};
        Desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        ThrowIfFailed(Device->CreateCommandQueue(&Desc, IID_PPV_ARGS(&Result.CommandQueue)));
    }

    {
        DXGI_SWAP_CHAIN_DESC1 Desc = {};
        Desc.Width = Width;
        Desc.Height = Height;
        Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.SampleDesc.Count = 1;
        Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        Desc.BufferCount = 2;
        Desc.Scaling = DXGI_SCALING_STRETCH;
        Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

        ThrowIfFailed(Factory->CreateSwapChainForHwnd(Result.CommandQueue, WindowHandle, &Desc, nullptr, nullptr, &Result.SwapChain));

        Result.CurrentFrameIndex = 0;
        ThrowIfFailed(Result.SwapChain->GetBuffer(0, IID_PPV_ARGS(&Result.FrameBuffers[0])));
        ThrowIfFailed(Result.SwapChain->GetBuffer(1, IID_PPV_ARGS(&Result.FrameBuffers[1])));
    }

    ThrowIfFailed(Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Result.CommandAllocator)));
    ThrowIfFailed(Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Result.CommandAllocator, 0, IID_PPV_ARGS(&Result.CommandList)));
    ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Result.Fence)));
    Result.FenceValue = 0;

    Result.RtvArena = Dx12ArenaCreate(Device, D3D12_HEAP_TYPE_DEFAULT,
                                      MegaBytes(50),
                                      D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES);

    Result.UploadArena = Dx12UploadArenaCreate(&Result, MegaBytes(300));
    
    Result.BufferArena = Dx12ArenaCreate(Device, D3D12_HEAP_TYPE_DEFAULT,
                                         MegaBytes(100),
                                         D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS);

    Result.TextureArena = Dx12ArenaCreate(Device, D3D12_HEAP_TYPE_DEFAULT,
                                          MegaBytes(300),
                                          D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES);

    Result.RtvHeap = Dx12DescriptorHeapCreate(&Result, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2);
    Result.DsvHeap = Dx12DescriptorHeapCreate(&Result, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    Result.ShaderDescHeap = Dx12DescriptorHeapCreate(&Result, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 50, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    
    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        Desc.Width = Width;
        Desc.Height = Height;
        Desc.DepthOrArraySize = 1;
        Desc.MipLevels = 1;
        Desc.Format = DXGI_FORMAT_D32_FLOAT;
        Desc.SampleDesc.Count = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
        Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE ClearValue = {};
        ClearValue.Format = Desc.Format;
        ClearValue.DepthStencil.Depth = 1;
        
        Result.DepthBuffer = Dx12CreateResource(&Result, &Result.RtvArena, &Desc,
                                                D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                &ClearValue);
        Dx12DescriptorAllocate(&Result.DsvHeap, &Result.DepthDescriptor, 0);
        Device->CreateDepthStencilView(Result.DepthBuffer, 0, Result.DepthDescriptor);
    }
    
    {
        Dx12DescriptorAllocate(&Result.RtvHeap, Result.FrameBufferDescriptors + 0, 0);
        Device->CreateRenderTargetView(Result.FrameBuffers[0], nullptr, Result.FrameBufferDescriptors[0]);
        Dx12DescriptorAllocate(&Result.RtvHeap, Result.FrameBufferDescriptors + 1, 0);
        Device->CreateRenderTargetView(Result.FrameBuffers[1], nullptr, Result.FrameBufferDescriptors[1]);
    }

    {

        {
            D3D12_ROOT_PARAMETER RootParameters[3] = {};

            D3D12_DESCRIPTOR_RANGE Table1Range[1] = {};
            {
                Table1Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                Table1Range[0].NumDescriptors = 1;
                Table1Range[0].BaseShaderRegister = 0;
                Table1Range[0].RegisterSpace = 0;
                Table1Range[0].OffsetInDescriptorsFromTableStart = 0;

                RootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[0].DescriptorTable.NumDescriptorRanges = ArrayCount(Table1Range);
                RootParameters[0].DescriptorTable.pDescriptorRanges = Table1Range;
                RootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }

            D3D12_DESCRIPTOR_RANGE Table2Range[2] = {};
            {
                Table2Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                Table2Range[0].NumDescriptors = 1;
                Table2Range[0].BaseShaderRegister = 1;
                Table2Range[0].RegisterSpace = 0;
                Table2Range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                Table2Range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
                Table2Range[1].NumDescriptors = 1;
                Table2Range[1].BaseShaderRegister = 1;
                Table2Range[1].RegisterSpace = 0;
                Table2Range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                RootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[1].DescriptorTable.NumDescriptorRanges = ArrayCount(Table2Range);
                RootParameters[1].DescriptorTable.pDescriptorRanges = Table2Range;
                RootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }

            D3D12_DESCRIPTOR_RANGE Table3Range[1] = {};
            {
                Table3Range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
                Table3Range[0].NumDescriptors = 1;
                Table3Range[0].BaseShaderRegister = 0;
                Table3Range[0].RegisterSpace = 0;
                Table3Range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

                RootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
                RootParameters[2].DescriptorTable.NumDescriptorRanges = ArrayCount(Table3Range);
                RootParameters[2].DescriptorTable.pDescriptorRanges = Table3Range;
                RootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            }
            
            D3D12_STATIC_SAMPLER_DESC StaticSamplerDesc = {};
            StaticSamplerDesc.Filter = D3D12_FILTER_ANISOTROPIC;
            StaticSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            StaticSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
            StaticSamplerDesc.MaxAnisotropy = 16.0f;
            StaticSamplerDesc.MinLOD = 0;
            StaticSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
            StaticSamplerDesc.ShaderRegister = 0;
            StaticSamplerDesc.RegisterSpace = 0;
            StaticSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
            
            D3D12_ROOT_SIGNATURE_DESC SignatureDesc = {};
            SignatureDesc.NumParameters = ArrayCount(RootParameters);
            SignatureDesc.pParameters = RootParameters;
            SignatureDesc.NumStaticSamplers = 1;
            SignatureDesc.pStaticSamplers = &StaticSamplerDesc;
            SignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
            
            ID3DBlob* SerializedRootSig = 0;
            ID3DBlob* ErrorBlob = 0;
            ThrowIfFailed(D3D12SerializeRootSignature(&SignatureDesc,
                                                      D3D_ROOT_SIGNATURE_VERSION_1_0,
                                                      &SerializedRootSig,
                                                      &ErrorBlob));
            ThrowIfFailed(Device->CreateRootSignature(0,
                                                      SerializedRootSig->GetBufferPointer(),
                                                      SerializedRootSig->GetBufferSize(),
                                                      IID_PPV_ARGS(&Result.ModelRootSignature)));

            if (SerializedRootSig)
            {
                SerializedRootSig->Release();
            }
            if (ErrorBlob)
            {
                ErrorBlob->Release();
            }
        }

        D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = {};
        Desc.pRootSignature = Result.ModelRootSignature;
        
        Desc.VS = Dx12LoadShader("ModelVsMain.shader");
        Desc.PS = Dx12LoadShader("ModelPsMain.shader");

        Desc.BlendState.RenderTarget[0].BlendEnable = true;
        Desc.BlendState.RenderTarget[0].LogicOpEnable = false;
        Desc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        Desc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
        Desc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        Desc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        Desc.SampleMask = 0xFFFFFFFF;

        Desc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
        Desc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        Desc.RasterizerState.FrontCounterClockwise = FALSE;
        Desc.RasterizerState.DepthClipEnable = TRUE;

        Desc.DepthStencilState.DepthEnable = TRUE;
        Desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        Desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        
        D3D12_INPUT_ELEMENT_DESC InputElementDescs[3] = {};
        InputElementDescs[0].SemanticName = "POSITION";
        InputElementDescs[0].SemanticIndex = 0;
        InputElementDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        InputElementDescs[0].InputSlot = 0;
        InputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

        InputElementDescs[1].SemanticName = "TEXCOORD";
        InputElementDescs[1].SemanticIndex = 0;
        InputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
        InputElementDescs[1].InputSlot = 0;
        InputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;

        InputElementDescs[2].SemanticName = "NORMAL";
        InputElementDescs[2].SemanticIndex = 0;
        InputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
        InputElementDescs[2].InputSlot = 0;
        InputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
        InputElementDescs[2].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
        
        Desc.InputLayout.pInputElementDescs = InputElementDescs;
        Desc.InputLayout.NumElements = ArrayCount(InputElementDescs);

        Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        Desc.NumRenderTargets = 1;
        Desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        Desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
        Desc.SampleDesc.Count = 1;
        
        ThrowIfFailed(Device->CreateGraphicsPipelineState(&Desc, IID_PPV_ARGS(&Result.ModelPipeline)));
    }

    Dx12CreateConstantBuffer(&Result, sizeof(transform_buffer_cpu),
                             &Result.SponzaTransformBuffer,
                             &Result.SponzaTransformDescriptor);

    for (i32 PlIndex = 0; PlIndex < NUM_POINT_LIGHTS; ++PlIndex)
    {
        Dx12CreateConstantBuffer(&Result, sizeof(transform_buffer_cpu),
                                 Result.PlTransformBuffers + PlIndex,
                                 Result.PlTransformDescriptors + PlIndex);
    }

    Dx12CreateConstantBuffer(&Result, sizeof(dir_light_buffer_cpu),
                             &Result.DirLightBuffer, &Result.LightDescriptor);

    {
        D3D12_RESOURCE_DESC Desc = {};
        Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        Desc.Width = sizeof(point_light_cpu) * 100;
        Desc.Height = 1;
        Desc.DepthOrArraySize = 1;
        Desc.MipLevels = 1;
        Desc.Format = DXGI_FORMAT_UNKNOWN;
        Desc.SampleDesc.Count = 1;
        Desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        Result.PointLightBuffer = Dx12CreateResource(&Result,
                                                     &Result.BufferArena,
                                                     &Desc,
                                                     D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
                                                     0);
        
        D3D12_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
        SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
        SrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        SrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        SrvDesc.Buffer.FirstElement = 0;
        SrvDesc.Buffer.NumElements = 100;
        SrvDesc.Buffer.StructureByteStride = sizeof(point_light_cpu);

        D3D12_CPU_DESCRIPTOR_HANDLE CpuDescriptor = {};
        Dx12DescriptorAllocate(&Result.ShaderDescHeap, &CpuDescriptor, 0);
        Device->CreateShaderResourceView(Result.PointLightBuffer, &SrvDesc, CpuDescriptor);
    }
    
    return Result;
}
