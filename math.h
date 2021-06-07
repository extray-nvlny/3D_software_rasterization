/* date = May 27th 2021 11:08 pm */

#ifndef MATH_H
#define MATH_H

#define PI 3.14159265

#define DEG_TO_RAD(x) (x*PI/180.0f)
#define RAD_TO_DEG(x) (x*180.0f/PI)

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


typedef struct v4 
{
    union
    {
        f32 e[4];
        struct 
        {
            f32 x,y,z,w;
        };
        struct 
        {
            f32 r,g,b,a;
        };
        struct
        {
            v3 xyz;
            f32 igonred;
        };
    };
}v4;


#define v3f(x,y,z) v3f_create(x,y,z)

v3 
v3f_create(f32 x,f32 y, f32 z)
{
    struct v3 result = {x,y,z};
    return result;
}


#define v4f(x,y,z,w) v4f_create(x,y,z,w)

v4 
v4f_create(f32 x,f32 y, f32 z,f32 w)
{
    struct v4 result = {x,y,z,w};
    return result;
}



typedef struct m4x4
{
    union
    {
        f32 e[4][4];
        struct
        {
            f32 m00, m01, m02, m03;
            f32 m10, m11, m12, m13;
            f32 m20, m21, m22, m23;
            f32 m30, m31, m32, m33;
        };
    };
}m4x4;


typedef struct m3x3
{
    union
    {
        f32 e[3][3];
        struct
        {
            f32 m00, m01, m02;
            f32 m10, m11, m12;
            f32 m20, m21, m22; 
        };
    };
}m3x3;


m4x4
mul_m4x4m4x4(m4x4 A, m4x4 B)
{
    m4x4 result = {0};
    
    for(u32 rows = 0;
        rows < 4;
        rows++)
    {
        for(u32 columns = 0;
            columns < 4;
            columns++)
        {
            for(u32 index = 0;
                index < 4;
                index++)
            {
                result.e[rows][columns] += A.e[rows][index] + B.e[index][columns];
            }
        }
    }
    
    return result;
}



v3 
mul_m4x4v3(m4x4 A, v3 b)
{
    v3 result = {0};
    
    result.x = b.x*A.e[0][0]+b.y*A.e[0][1]+b.z*A.e[0][2] + 1.0f*A.e[0][3];
    result.y = b.x*A.e[1][0]+b.y*A.e[1][1]+b.z*A.e[1][2] + 1.0f*A.e[1][3];
    result.z = b.x*A.e[2][0]+b.y*A.e[2][1]+b.z*A.e[2][2] + 1.0f*A.e[2][3];
    
    return result;
}



v4
mul_m4x4v4(m4x4 A, v4 b)
{
    v4 result = {0};
    
    result.x = b.x*A.e[0][0]+b.y*A.e[0][1]+b.z*A.e[0][2]+b.w*A.e[0][3];
    result.y = b.x*A.e[1][0]+b.y*A.e[1][1]+b.z*A.e[1][2]+b.w*A.e[1][3];
    result.z = b.x*A.e[2][0]+b.y*A.e[2][1]+b.z*A.e[2][2]+b.w*A.e[2][3];
    result.w = b.x*A.e[3][0]+b.y*A.e[3][1]+b.z*A.e[3][2]+b.w*A.e[3][3];
    
    return result;
}

v3
mul_f32v3(f32 x, v3 A)
{
    v3 result = {0};
    
    result.x = A.x * x;
    result.y = A.y * x;
    result.z = A.z * x;
    
    return result;
}


v4
mul_f32v4(f32 x, v4 A)
{
    v4 result = {0};
    
    result.x = A.x * x;
    result.y = A.y * x;
    result.z = A.z * x;
    result.z = 1.0f;
    
    return result;
}


v3
add_v3v3(v3 A, v3 B)
{
    v3 result = {0};
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    
    return result;
}


v4
add_v4v4(v4 A, v4 B)
{
    v4 result = {0};
    
    result.x = A.x + B.x;
    result.y = A.y + B.y;
    result.z = A.z + B.z;
    result.w = 1.0f;
    
    return result;
}


v3
subtract_v3v3(v3 A, v3 B)
{
    v3 result = {0};
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    result.z = A.z - B.z;
    
    return result;
}

v2
subtract_v2v2(v2 A, v2 B)
{
    v2 result = {0};
    
    result.x = A.x - B.x;
    result.y = A.y - B.y;
    
    return result;
}


f32 
dot_product_3(v3 A,v3 B)
{
    f32 result = 0;
    
    result = A.x*B.x + A.y*B.y + A.z*B.z;
    
    return result;
}


f32 
dot_product_2(v2 A,v2 B)
{
    f32 result = 0;
    
    result = A.x*B.x + A.y*B.y;
    
    return result;
}


v3 
lerp_v3(v3 A, v3 B, f32 t)
{
    v3 result = {0};
    
    result = add_v3v3(mul_f32v3((1.0f - t),A),mul_f32v3(t,B));
    
    return result;
}


v4 
lerp_v4(v4 A, v4 B, f32 t)
{
    v4 result = {0};
    
    result = add_v4v4(mul_f32v4((1.0f - t),A),mul_f32v4(t,B));
    
    return result;
}


v3
cross_product_3(v3 a, v3 b)
{
    v3 result = {0};
    // 2 3 1 and X shape technique
    result.x = a.y*b.z - a.z*b.y;
    result.y = a.z*b.x - b.z*a.x;
    result.z = a.x*b.y - b.x*a.y;
    
    return result;
}

v3
normalize_v3(v3 a)
{
    v3 result = {0};
    
    f32 length = sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
    
    result.x = a.x / length;
    result.y = a.y / length;
    result.z = a.z / length;
    
    return result;
}

#endif //MATH_H
