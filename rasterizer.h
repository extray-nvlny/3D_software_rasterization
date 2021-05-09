#include "platform.h"
#include <math.h>

typedef struct v3 
{
    union
    {
        f32 e[3];
        struct 
        {
            f32 x,y,z;
        };
        struct 
        {
            f32 r,g,b;
        };
    };
}v3;

#define v3f(x,y,z) v3f_create(x,y,z)

v3 
v3f_create(f32 x,f32 y, f32 z)
{
    struct v3 result = {x,y,z};
    return result;
}

