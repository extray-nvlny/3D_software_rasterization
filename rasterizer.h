#include "platform.h"
#include <math.h>
#include "math.h"
#include "assets.c"

#define MAX_POLYS    128
#define MAX_VERTICES 128

// TODO(shvayko): Are they constants?
#define VIEWPORT_WIDTH  800
#define VIEWPORT_HEIGHT 600

typedef struct Object
{
    v3 world_p;
    
    v3 vertices[MAX_VERTICES];
    u32 vertices_count;
    
    Poly polys[MAX_POLYS];
    u32 poly_count;
    
    Bitmap *texture;
}Object;
global Object g_objects[1024];
global u32 g_object_count;

typedef struct PolySelf
{
    Vertex vertices[3];
    
    struct PolySelf *next;
    struct PolySelf *prev;
}PolySelf;

typedef struct RenderList
{
    
    PolySelf poly_pointers[128];
    
    // NOTE(shvayko): Actual data
    PolySelf poly_data[128];
    
    u32 poly_count;
}RenderList;

global RenderList g_render_list;

