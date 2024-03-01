#if !defined(GRAPHICS_MATH_H)

global float Pi32 = 3.14159265359f;

#include <xmmintrin.h>
#include <emmintrin.h>
#include <pmmintrin.h>
#include <tmmintrin.h>
#include <smmintrin.h>
#include <nmmintrin.h>
#include <glm/glm.hpp>

struct v4
{
    union
    {
        struct
        {
            float x, y, z, w;
        };

        struct
        {
            float r, g, b, a;
        };

        struct
        {
            glm::vec3 xyz;
            float Ignored0;
        };

        struct
        {
            float Ignored3;
            glm::vec3 yzw;
        };
        
        float e[4];
    };

    float& operator[](i64 Index)
    {
        return e[Index];
    }
};

struct m4
{
    union
    {
        v4 v[4];
        float e[16];
    };
        
    v4& operator[](i64 Index)
    {
        return v[Index];
    }
};

#define GRAPHICS_MATH_H
#endif
