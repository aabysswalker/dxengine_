
char* CombineStrings(const char* Str1, const char* Str2)
{
    char* Result = 0;
    size_t Length1 = strlen(Str1);
    size_t Length2 = strlen(Str2);
    Result = (char*)malloc(sizeof(char) * (Length1 + Length2 + 1));
    memcpy(Result, Str1, Length1 * sizeof(char));
    memcpy(Result + Length1, Str2, Length2 * sizeof(char));
    Result[Length1 + Length2] = 0;

    return Result;
}

model AssetCreateCube(dx12_rasterizer* Dx12Rasterizer)
{
    model Result = {};

    {
        local_global int Texel = 0xFFFFFFFF;
        
        Result.NumTextures = 1;
        Result.TextureArray = (texture*)malloc(sizeof(texture));
        *Result.TextureArray = {};
        Result.TextureArray[0].Width = 1;
        Result.TextureArray[0].Height = 1;
        Result.TextureArray[0].Texels = &Texel;
        
        Dx12CreateTexture(Dx12Rasterizer, Result.TextureArray[0].Width, Result.TextureArray[0].Height, (uint8_t*)Result.TextureArray[0].Texels,
                          &Result.TextureArray[0].GpuTexture, &Result.TextureArray[0].GpuDescriptor);
    }

    {
        local_global vertex ModelVertices[] =
            {
                // NOTE: Front Face
                { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(0, 0, 1) },
                { glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(1, 0), glm::vec3(0, 0, 1) },
                { glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(1, 1), glm::vec3(0, 0, 1) },
                { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0, 1), glm::vec3(0, 0, 1) },

                // NOTE: Back Face
                { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(0, 0), glm::vec3(0, 0, -1) },
                { glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1, 0), glm::vec3(0, 0, -1) },
                { glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1, 1), glm::vec3(0, 0, -1) }, 
                { glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(0, 1), glm::vec3(0, 0, -1) },

                // NOTE: Left Face
                { glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(-1, 0, 0) },
                { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1, 0), glm::vec3(-1, 0, 0) },
                { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1, 1), glm::vec3(-1, 0, 0) },
                { glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(0, 1), glm::vec3(-1, 0, 0) },

                // NOTE: Right Face
                { glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(1, 0, 0) },
                { glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1, 0), glm::vec3(1, 0, 0) },
                { glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1, 1), glm::vec3(1, 0, 0) },
                { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0, 1), glm::vec3(1, 0, 0) },

                // NOTE: Top Face
                { glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(0, 1, 0) },
                { glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec2(1, 0), glm::vec3(0, 1, 0) },
                { glm::vec3(0.5f, 0.5f, 0.5f), glm::vec2(1, 1), glm::vec3(0, 1, 0) },
                { glm::vec3(0.5f, 0.5f, -0.5f), glm::vec2(0, 1), glm::vec3(0, 1, 0) },

                // NOTE: Bottom Face
                { glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0, 0), glm::vec3(0, -1, 0) },
                { glm::vec3(0.5f, -0.5f, -0.5f), glm::vec2(0, 1), glm::vec3(0, -1, 0) },
                { glm::vec3(0.5f, -0.5f, 0.5f), glm::vec2(1, 1), glm::vec3(0, -1, 0) },
                { glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec2(1, 0), glm::vec3(0, -1, 0) },
            };

        local_global int ModelIndices[] =
            {
                // NOTE: Front Face
                0, 1, 2,
                2, 3, 0,

                // NOTE: Back Face
                4, 7, 6,
                6, 5, 4,

                // NOTE: Left face
                8, 9, 10,
                10, 11, 8,

                // NOTE: Right face
                12, 13, 14,
                14, 15, 12,

                // NOTE: Top face
                16, 17, 18,
                18, 19, 16,

                // NOTE: Bottom face
                20, 21, 22,
                22, 23, 20,
            };

        
        Result.VertexCount = ArrayCount(ModelVertices);
        Result.IndexCount = ArrayCount(ModelIndices);

        Result.NumMeshes = 1;
        Result.MeshArray = (mesh*)malloc(sizeof(mesh));
        {
            mesh Mesh = {};
            Mesh.IndexOffset = 0;
            Mesh.IndexCount = Result.IndexCount;
            Mesh.VertexOffset = 0;
            Mesh.VertexCount = Result.VertexCount;
            Mesh.TextureId = 0;
            Result.MeshArray[0] = Mesh;
        }

        Result.VertexArray = ModelVertices;
        Result.IndexArray = ModelIndices;
        Result.GpuVertexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.VertexCount * sizeof(vertex), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, ModelVertices);
        Result.GpuIndexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.IndexCount * sizeof(int), D3D12_RESOURCE_STATE_INDEX_BUFFER, ModelIndices);
    }
    
    return Result;
}

model AssetLoadModel(dx12_rasterizer* Dx12Rasterizer, const char* FolderPath, const char* FileName)
{
    model Result = {};

    std::string FilePath = "data/sponza/Sponza.gltf";
    
    Assimp::Importer Importer;
    const aiScene* Scene = Importer.ReadFile(FilePath, aiProcess_Triangulate | aiProcess_OptimizeMeshes);
    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
    {
        const char* Error = Importer.GetErrorString();
        InvalidCodePath;
    }
    
    int* TextureMappingTable = (int*)malloc(sizeof(int) * Scene->mNumMaterials);
    for (int MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId];

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            TextureMappingTable[MaterialId] = Result.NumTextures;
            Result.NumTextures += 1;
        }
        else
        {
            TextureMappingTable[MaterialId] = 0xFFFFFFFF;
        }
    }

    Result.TextureArray = (texture*)malloc(sizeof(texture) * Result.NumTextures);

    int CurrTextureId = 0;
    for (int MaterialId = 0; MaterialId < Scene->mNumMaterials; ++MaterialId)
    {
        aiMaterial* CurrMaterial = Scene->mMaterials[MaterialId];

        if (CurrMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        {
            aiString TextureName = {};
            CurrMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &TextureName);

            texture* CurrTexture = Result.TextureArray + CurrTextureId;

            char* TexturePath = CombineStrings("data/sponza/", TextureName.C_Str());
            
            i32 NumChannels = 0;
            int* UnFlippedTexels = (int*)stbi_load(TexturePath, &CurrTexture->Width, &CurrTexture->Height, &NumChannels, 4);

            CurrTexture->Texels = (int*)malloc(sizeof(int) * CurrTexture->Width * CurrTexture->Height);

            for (int Y = 0; Y < CurrTexture->Height; ++Y)
            {
                for (int X = 0; X < CurrTexture->Width; ++X)
                {
                    int PixelId = Y * CurrTexture->Width + X;
                    CurrTexture->Texels[PixelId] = UnFlippedTexels[(CurrTexture->Height - Y - 1) * CurrTexture->Width + X];
                }
            }

            // NOTE: Копюємо дані до Upload Heap
            Dx12CreateTexture(Dx12Rasterizer, CurrTexture->Width, CurrTexture->Height, (uint8_t*)CurrTexture->Texels,
                              &CurrTexture->GpuTexture, &CurrTexture->GpuDescriptor);
            
            stbi_image_free(UnFlippedTexels);
            
            CurrTextureId += 1;
        }
    }
    
    Result.NumMeshes = Scene->mNumMeshes;
    Result.MeshArray = (mesh*)malloc(sizeof(mesh) * Result.NumMeshes);
    for (int MeshId = 0; MeshId < Result.NumMeshes; ++MeshId)
    {
        aiMesh* SrcMesh = Scene->mMeshes[MeshId];
        mesh* DstMesh = Result.MeshArray + MeshId;

        DstMesh->TextureId = TextureMappingTable[SrcMesh->mMaterialIndex];
        DstMesh->IndexOffset = Result.IndexCount;
        DstMesh->VertexOffset = Result.VertexCount;
        DstMesh->IndexCount = SrcMesh->mNumFaces * 3;
        DstMesh->VertexCount = SrcMesh->mNumVertices;

        Result.IndexCount += DstMesh->IndexCount;
        Result.VertexCount += DstMesh->VertexCount;
    }

    Result.VertexArray = (vertex*)malloc(sizeof(vertex) * Result.VertexCount);
    Result.IndexArray = (int*)malloc(sizeof(int) * Result.IndexCount);

    float MinDistAxis = FLT_MAX;
    float MaxDistAxis = FLT_MIN;
    for (int MeshId = 0; MeshId < Result.NumMeshes; ++MeshId)
    {
        aiMesh* SrcMesh = Scene->mMeshes[MeshId];
        mesh* DstMesh = Result.MeshArray + MeshId;

        for (int VertexId = 0; VertexId < DstMesh->VertexCount; ++VertexId)
        {
            vertex* CurrVertex = Result.VertexArray + DstMesh->VertexOffset + VertexId;
            CurrVertex->Pos = glm::vec3(SrcMesh->mVertices[VertexId].x,
                                 SrcMesh->mVertices[VertexId].y,
                                 SrcMesh->mVertices[VertexId].z);

            MinDistAxis = std::min(MinDistAxis, std::min(CurrVertex->Pos.x, std::min(CurrVertex->Pos.y, CurrVertex->Pos.z)));
            MaxDistAxis = std::max(MaxDistAxis, std::max(CurrVertex->Pos.x, std::max(CurrVertex->Pos.y, CurrVertex->Pos.z)));

            CurrVertex->Normal = glm::vec3(SrcMesh->mNormals[VertexId].x,
                                    SrcMesh->mNormals[VertexId].y,
                                    SrcMesh->mNormals[VertexId].z);
            
            if (SrcMesh->mTextureCoords[0])
            {
                CurrVertex->Uv = glm::vec2(SrcMesh->mTextureCoords[0][VertexId].x,
                                    SrcMesh->mTextureCoords[0][VertexId].y);
            }
            else
            {
                CurrVertex->Uv = glm::vec2(0, 0);
            }
        }

        for (int FaceId = 0; FaceId < SrcMesh->mNumFaces; ++FaceId)
        {
            int* CurrIndices = Result.IndexArray + DstMesh->IndexOffset + FaceId * 3;

            CurrIndices[0] = SrcMesh->mFaces[FaceId].mIndices[0];
            CurrIndices[1] = SrcMesh->mFaces[FaceId].mIndices[1];
            CurrIndices[2] = SrcMesh->mFaces[FaceId].mIndices[2];
        }
    }
    
    for (int VertexId = 0; VertexId < Result.VertexCount; ++VertexId)
    {
        vertex* CurrVertex = Result.VertexArray + VertexId;

        // NOTE: Результат є між [0, 1]
        CurrVertex->Pos = (CurrVertex->Pos - glm::vec3(MinDistAxis)) / (MaxDistAxis - MinDistAxis);
        CurrVertex->Pos = CurrVertex->Pos - glm::vec3(0.5f);
    }

    Result.GpuVertexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.VertexCount * sizeof(vertex), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, Result.VertexArray);
    Result.GpuIndexBuffer = Dx12CreateBufferAsset(Dx12Rasterizer, Result.IndexCount * sizeof(int), D3D12_RESOURCE_STATE_INDEX_BUFFER, Result.IndexArray);
    
    return Result;
}
