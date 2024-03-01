#if !defined(WIN32_GRAPHICS_H)

#include <stdint.h>
#include <stddef.h>
#include <float.h>
#include <stdio.h>
#include <glm/gtc/matrix_transform.hpp>
typedef int32_t i32;
typedef int64_t i64;

typedef uint16_t u16;

#define global static
#define local_global static

#define snprintf _snprintf_s
#define Assert(Expression) if (!(Expression)) {__debugbreak();}
#define InvalidCodePath Assert(!"Invalid Code Path")
#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

#define KiloBytes(Val) ((Val)*1024LL)
#define MegaBytes(Val) (KiloBytes(Val)*1024LL)
#define GigaBytes(Val) (MegaBytes(Val)*1024LL)
#define TeraBytes(Val) (GigaBytes(Val)*1024LL)

#include "graphics_math.h"
#include "dx12_rasterizer.h"
#include "assets.h"
#include <glm/glm.hpp>

class Camera
{
public:
    int PrevMouseDown;
    glm::vec2 PrevMousePos;

    float Yaw;
    float Pitch;
    
    glm::vec3 Pos;
};

enum rasterizer_type
{
    RasterizerType_None,

    RasterizerType_Software,
    RasterizerType_Dx12,
};

struct global_state
{
    int IsRunning;
    HWND WindowHandle;
    
    float CurrTime;

    int WDown;
    int ADown;
    int SDown;
    int DDown;
    Camera camera;

    model CubeModel;
    model DuckModel;
    model SponzaModel;

    rasterizer_type RasterizerType;
    dx12_rasterizer Dx12Rasterizer;
};

#define WIN32_GRAPHICS_H
#endif
