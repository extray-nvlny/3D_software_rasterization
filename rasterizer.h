#include "platform.h"
#include <math.h>


typedef struct v2 
{
    union
    {
        f32 e[2];
        struct 
        {
            f32 x,y;
        };
        struct 
        {
            f32 u,v;
        };
    };
    
}v2;

#define v2f(x,y) v2f_create(x,y)

v2 
v2f_create(f32 x,f32 y)
{
    struct v2 result = {x,y};
    return result;
}


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
        struct
        {
            v2 xy;
            f32 igonred;
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

v3
mul_f32v3(f32 x, v3 A)
{
    v3 result;
    
    result.x = A.x * x;
    result.y = A.y * x;
    result.z = A.z * x;
    
    return result;
}

v3
add_v3v3(v3 A, v3 B)
{
    v3 result;
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    
    return result;
}

v3
subtract_v3v3(v3 A, v3 B)
{
    v3 result;
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    
    return result;
}

v2
subtract_v2v2(v2 A, v2 B)
{
    v2 result;
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    
    return result;
}


f32 
dot_product_3(v3 A,v3 B)
{
    f32 result;
    
    result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return result;
}


f32 
dot_product_2(v2 A,v2 B)
{
    f32 result;
    
    result = A.x*B.x + A.y*B.y;
    
    return result;
}


v3 
lerp_v3(v3 A, v3 B, f32 t)
{
    v3 result;
    
    result = add_v3v3(mul_f32v3((1.0f - t),A),mul_f32v3(t,B));
    
    return result;
}


typedef struct 
{
    void *memory;
    u32 file_size;
}FileContent;

FileContent win_read_file(char *filename);