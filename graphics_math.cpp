// Refactor consider using glm

uint64_t Align(uint64_t Location, uint64_t Alignment)
{
    uint64_t Result = (Location + (Alignment - 1)) & (~(Alignment - 1));
    return Result;
}

v4 V4(float X, float Y, float Z, float W)
{
    v4 Result = {};
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return Result;
}

v4 operator+(v4 A, v4 B)
{
    v4 Result = {};
    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;
    return Result;
}

v4 operator*(v4 A, float B)
{
    v4 Result = {};
    Result.x = A.x * B;
    Result.y = A.y * B;
    Result.z = A.z * B;
    Result.w = A.w * B;
    return Result;
}

m4 IdentityM4()
{
    m4 Result = {};
    Result.v[0].x = 1.0f;
    Result.v[1].y = 1.0f;
    Result.v[2].z = 1.0f;
    Result.v[3].w = 1.0f;
    return Result;
}

v4 operator*(m4 A, v4 B)
{
    v4 Result = A.v[0] * B.x + A.v[1] * B.y + A.v[2] * B.z + A.v[3] * B.w;
    return Result;
}

m4 operator*(m4 A, m4 B)
{
    m4 Result = {};
    Result.v[0] = A * B.v[0];
    Result.v[1] = A * B.v[1];
    Result.v[2] = A * B.v[2];
    Result.v[3] = A * B.v[3];
    return Result;
}

m4 Transpose(m4 A)
{
    m4 Result = {};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            Result[i][j] = A[j][i];
        }
    }

    return Result;
}

float Determinant3x3(glm::vec3 ColA, glm::vec3 ColB, glm::vec3 ColC)
{
    return glm::determinant(glm::mat3(ColA, ColB, ColC));
}

float Determinant(m4 A)
{
    float Result = (A[0][0] * Determinant3x3(A[1].yzw, A[2].yzw, A[3].yzw) -
                  A[0][1] * Determinant3x3(glm::vec3(A[1].x, A[1].z, A[1].w), glm::vec3(A[2].x, A[2].z, A[2].w), glm::vec3(A[3].x, A[3].z, A[3].w)) +
                  A[0][2] * Determinant3x3(glm::vec3(A[1].x, A[1].y, A[1].w), glm::vec3(A[2].x, A[2].y, A[2].w), glm::vec3(A[3].x, A[3].y, A[3].w)) -
                  A[0][3] * Determinant3x3(A[1].xyz, A[2].xyz, A[3].xyz));
    return Result;
}

m4 Inverse(m4 A)
{
    float OneOverDeterminant = Determinant(A);
    Assert(OneOverDeterminant != 0.0f);
    OneOverDeterminant = 1.0f / OneOverDeterminant;
    
    m4 Result = {};
    Result[0][0] = +Determinant3x3(A[1].yzw, A[2].yzw, A[3].yzw) * OneOverDeterminant;
    Result[0][1] = -Determinant3x3(A[0].yzw, A[2].yzw, A[3].yzw) * OneOverDeterminant;
    Result[0][2] = +Determinant3x3(A[0].yzw, A[1].yzw, A[3].yzw) * OneOverDeterminant;
    Result[0][3] = -Determinant3x3(A[0].yzw, A[1].yzw, A[2].yzw) * OneOverDeterminant;

    Result[1][0] = -Determinant3x3(glm::vec3(A[1].x, A[1].z, A[1].w), glm::vec3(A[2].x, A[2].z, A[2].w), glm::vec3(A[3].x, A[3].z, A[3].w)) * OneOverDeterminant;
    Result[1][1] = +Determinant3x3(glm::vec3(A[0].x, A[0].z, A[0].w), glm::vec3(A[2].x, A[2].z, A[2].w), glm::vec3(A[3].x, A[3].z, A[3].w)) * OneOverDeterminant;
    Result[1][2] = -Determinant3x3(glm::vec3(A[0].x, A[0].z, A[0].w), glm::vec3(A[1].x, A[1].z, A[1].w), glm::vec3(A[3].x, A[3].z, A[3].w)) * OneOverDeterminant;
    Result[1][3] = +Determinant3x3(glm::vec3(A[0].x, A[0].z, A[0].w), glm::vec3(A[1].x, A[1].z, A[1].w), glm::vec3(A[2].x, A[2].z, A[2].w)) * OneOverDeterminant;

    Result[2][0] = +Determinant3x3(glm::vec3(A[1].x, A[1].y, A[1].w), glm::vec3(A[2].x, A[2].y, A[2].w), glm::vec3(A[3].x, A[3].y, A[3].w)) * OneOverDeterminant;
    Result[2][1] = -Determinant3x3(glm::vec3(A[0].x, A[0].y, A[0].w), glm::vec3(A[2].x, A[2].y, A[2].w), glm::vec3(A[3].x, A[3].y, A[3].w)) * OneOverDeterminant;
    Result[2][2] = +Determinant3x3(glm::vec3(A[0].x, A[0].y, A[0].w), glm::vec3(A[1].x, A[1].y, A[1].w), glm::vec3(A[3].x, A[3].y, A[3].w)) * OneOverDeterminant;
    Result[2][3] = -Determinant3x3(glm::vec3(A[0].x, A[0].y, A[0].w), glm::vec3(A[1].x, A[1].y, A[1].w), glm::vec3(A[2].x, A[2].y, A[2].w)) * OneOverDeterminant;

    Result[3][0] = -Determinant3x3(A[1].xyz, A[2].xyz, A[3].xyz) * OneOverDeterminant;
    Result[3][1] = +Determinant3x3(A[0].xyz, A[2].xyz, A[3].xyz) * OneOverDeterminant;
    Result[3][2] = -Determinant3x3(A[0].xyz, A[1].xyz, A[3].xyz) * OneOverDeterminant;
    Result[3][3] = +Determinant3x3(A[0].xyz, A[1].xyz, A[2].xyz) * OneOverDeterminant;

    return Result;
}

m4 ScaleMatrix(float X, float Y, float Z)
{
    m4 Result = IdentityM4();
    Result.v[0].x = X;
    Result.v[1].y = Y;
    Result.v[2].z = Z;
    return Result;
}

m4 RotationMatrix(float X, float Y, float Z)
{
    m4 Result = {};

    m4 RotateX = IdentityM4();
    RotateX.v[1].y = cos(X);
    RotateX.v[2].y = -sin(X);
    RotateX.v[1].z = sin(X);
    RotateX.v[2].z = cos(X);

    m4 RotateY = IdentityM4();
    RotateY.v[0].x = cos(Y);
    RotateY.v[2].x = -sin(Y);
    RotateY.v[0].z = sin(Y);
    RotateY.v[2].z = cos(Y);

    m4 RotateZ = IdentityM4();
    RotateZ.v[0].x = cos(Z);
    RotateZ.v[1].x = -sin(Z);
    RotateZ.v[0].y = sin(Z);
    RotateZ.v[1].y = cos(Z);

    Result = RotateZ * RotateY * RotateX;
    return Result;
}

m4 TranslationMatrix(float X, float Y, float Z)
{
    m4 Result = IdentityM4();
    Result.v[3].xyz = glm::vec3(X, Y, Z);
    return Result;
}

m4 TranslationMatrix(glm::vec3 Pos)
{
    m4 Result = TranslationMatrix(Pos.x, Pos.y, Pos.z);
    return Result;
}

m4 PerspectiveMatrix(float Fov, float AspectRatio, float NearZ, float FarZ)
{
    // NOTE: Ми очікуємо що Fov є у градусів
    m4 Result = {};

    float FovRadians = (Fov / 360.0f) * 2.0f * Pi32;
    
    Result.v[0].x = 1.0f / (AspectRatio * tan(FovRadians * 0.5f));
    Result.v[1].y = 1.0f / (tan(FovRadians * 0.5f));
    Result.v[2].z = -FarZ / (NearZ - FarZ);
    Result.v[3].z = NearZ * FarZ / (NearZ - FarZ);
    Result.v[2].w = 1.0f;

    return Result;
}
