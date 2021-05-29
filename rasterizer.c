#include "rasterizer.h"
#include "math.h"
#include "assets.c"
#include "texture.c"
#include "camera.c"

#define MAX_POLYS    128
#define MAX_VERTICES 128

#define VIEWPORT_WIDTH  800
#define VIEWPORT_HEIGHT 600

typedef struct Vertex
{
    v3 p;
    v3 color;
}Vertex;

typedef struct Poly
{
    Vertex vertex[3];
}Poly;

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

void
reset_render_list(RenderList *render_list)
{
    render_list->poly_count = 0;
}

void
local_to_world_object(Object *object)
{
    // TODO(shvayko): Use matrix  here
    for(u32 vertex_index = 0;
        vertex_index < object->vertices_count;
        vertex_index++)
    {
        v3 *current_vertex = &object->vertices[vertex_index];
        *current_vertex = add_v3v3(object->world_p, *current_vertex);
    }
}

/*
void
world_to_camera(Camera *camera ,Object *object)
{
    for(u32 vertex_index = 0;
        vertex_index < object->vertices_count;
        vertex_index++)
    {
        
        v3 *current_vertex =  &object->vertices[vertex_index];
        *current_vertex = mul_m4x4v4(camera->camera_matrix, *current_vertex);
    }
}
*/

m4x4
build_perspective_projection_matrix(f32 width,f32 height,f32 fov, 
                                    f32 near,f32 far)
{   
    // NOTE(shvayko): near specifies the distance from the viewer to the near clipping plane (always positive). 
    // NOTE(shvayko): far specifies the distance from the viewer to the far clipping plane (always positive). 
    
    f32 ar = width / height;
    f32 tangent = tanf(DEG_TO_RAD(fov*0.5f));
    f32 d = (-near-far) / (near-far);
    f32 e = (2.0f*far*near) / (near - far);
    
    // NOTE(shvatko): ROW MAJOR
    m4x4 matrix = 
    {
        1.0f/(tangent*ar),0.0f,0.0f,0.0f,
        0.0f,1.0f / tangent,0.0f,0.0f,
        0.0f,0.0f,  d, e,
        0.0f,0.0f,  1.0f,0.0f,
    };
    
#if 0
    v4 test  = mul_m4x4v4(matrix,v4f(0.0f,0.0f, near,1.0f));
    v4 test0 = mul_m4x4v4(matrix,v4f(0.0f,0.0f, far, 1.0f));
    
    v3 ndc  = v3f(test.x / test.w,test.y / test.w,test.z / test.w);
    f32 w = test.z / test.w;
    v3 ndc0 = v3f(test0.x / test0.w,test0.y / test0.w,test0.z / test0.w);
    f32 wo = test0.z / test0.w;
#endif
    
    return matrix;
}

void
perspective_divide(const v4 *clip_v0, const v4 *clip_v1, const v4 *clip_v2,
                   v3 *ndc_v0,v3 *ndc_v1, v3 *ndc_v2)
{
    *ndc_v0 = v3f(clip_v0->x / clip_v0->w,clip_v0->y / clip_v0->w,clip_v0->z / clip_v0->w);
    *ndc_v1 = v3f(clip_v1->x / clip_v1->w,clip_v1->y / clip_v1->w,clip_v1->z / clip_v1->w);
    *ndc_v2 = v3f(clip_v2->x / clip_v2->w,clip_v2->y / clip_v2->w,clip_v2->z / clip_v2->w);
}

void
camera_to_clip(m4x4 projection_matrix,v4 *out_vertex)
{
    ASSERT(out_vertex->w == 1.0f);
    *out_vertex = mul_m4x4v4(projection_matrix,*out_vertex);
}

void
viewport(const v3 *ndc_v0,const v3 *ndc_v1,const v3 *ndc_v2,
         v3 *screen_v0,v3 *screen_v1,v3 *screen_v2)
{
    screen_v0->x = (ndc_v0->x + 1.0f)*VIEWPORT_WIDTH*0.5f;
    screen_v0->y = (ndc_v0->y + 1.0f)*VIEWPORT_HEIGHT*0.5f;
    screen_v0->z = ndc_v0->z;
    
    screen_v1->x = (ndc_v1->x + 1.0f)*VIEWPORT_WIDTH*0.5f;
    screen_v1->y = (ndc_v1->y + 1.0f)*VIEWPORT_HEIGHT*0.5f;
    screen_v1->z = ndc_v1->z;
    
    screen_v2->x = (ndc_v2->x + 1.0f)*VIEWPORT_WIDTH*0.5f;
    screen_v2->y = (ndc_v2->y + 1.0f)*VIEWPORT_HEIGHT*0.5f;
    screen_v2->z = ndc_v2->z;
}

u32
rgb1_to_rgb255(v3 A)
{
    u32 result;
    
    result = (((s32)(A.r*255.0f) << 16) |
              ((s32)(A.g*255.0f) << 8) | 
              ((s32)(A.b*255.0f)));
    
    return result;
}

void
barycentric(v3 A, v3 B, v3 C, v3 p, f32 *u, f32 *v, f32 *w)
{
    
    v2 v0 = subtract_v2v2(B.xy,A.xy);
    v2 v1 = subtract_v2v2(C.xy,A.xy);
    v2 v2 = subtract_v2v2(p.xy,A.xy);
    
    // v0 = B-A   v1 = C-A    v2 = p-A
    f32 d00 = dot_product_2(v0,v0);
    f32 d01 = dot_product_2(v0,v1);
    f32 d11 = dot_product_2(v1,v1);
    f32 d20 = dot_product_2(v2,v0);
    f32 d21 = dot_product_2(v2,v1);
    
    f32 denominator = d00*d11 - d01 * d01;
    
    *v = (d11*d20-d01*d21) / denominator;
    *w = (d00*d21-d01*d20) / denominator;
    *u = 1.0f - *v - *w;
}

// NOTE(shvayko): draw_scanline used only in draw_flat_x_tri funtions
void
draw_scanline(AppBackbuffer *backbuffer,s32 x0,s32 x1,s32 y,
              Vertex v0,Vertex v1,Vertex v2)
{
    u32 *pixel = (u32*)((u8*)backbuffer->memory + x0 * 4 + backbuffer->stride * y);
    
    f32 u,v,w;
    u = v = w = 0;
    for(s32 x = x0;
        x < x1;
        x++)
    {
        barycentric(v0.p,v1.p,v2.p,v3f(x,y,0),&u,&v,&w);
        v3 v3_color = add_v3v3(add_v3v3(mul_f32v3(u,v0.color),mul_f32v3(v,v1.color)),mul_f32v3(w,v2.color));
        u32 color = rgb1_to_rgb255(v3_color);
        *pixel++ = color;
    }
}

void
draw_flat_bottom_tri(AppBackbuffer *backbuffer, Vertex v0, Vertex v1, Vertex v2)
{
    
    // NOTE(shvayko):CLOCKWISE vertices order 
    
    // NOTE(shvayko): TOP-LEFT FILL CONVENTION 
    //                A top edge, is an edge that is exactly horizontal and is above the             //                other edges.
    //                A left edge, is an edge that is not exactly horizontal and is on the //                left side of the triangle. 
    
    f32 height =  (v2.p.y - v0.p.y);
    f32 dxy_left  = (v2.p.x - v0.p.x) / height;
    f32 dxy_right = (v1.p.x - v0.p.x) / height;
    
    f32 x_start = v0.p.x;
    f32 x_end   = v0.p.x;
    
    s32 y_start = (s32)ceil(v0.p.y - 0.5f);
    s32 y_end   = (s32)ceil(v2.p.y - 0.5f);
    
    for(s32 y = y_start;
        y <= y_end;
        y++)
    {
        if(!((y+1) <= y_end))
        {
            int stop = 6;
        }
        
        x_start = v0.p.x + (y-y_start)*dxy_left;
        x_end   = v0.p.x + (y-y_start)*dxy_right;
        
        s32 x0 = (s32)ceil(x_start - 0.5f);
        s32 x1 = (s32)ceil(x_end - 0.5f);
        draw_scanline(backbuffer, x0, x1, y, v0,v1,v2);
    }
}

void
draw_flat_top_tri(AppBackbuffer *backbuffer, Vertex v0, Vertex v1, Vertex v2)
{
    // NOTE(shvayko):CLOCKWISE vertices order 
    
    // NOTE(shvayko): TOP-LEFT FILL CONVENTION 
    //                A top edge, is an edge that is exactly horizontal and is above the             //                other edges.
    //                A left edge, is an edge that is not exactly horizontal and is on the //                left side of the triangle. 
    
    f32 height =  (v1.p.y - v0.p.y);
    f32 dxy_left  = (v1.p.x - v2.p.x) / height;
    f32 dxy_right = (v1.p.x - v0.p.x) / height;
    
    f32 x_start = v2.p.x;
    f32 x_end   = v0.p.x;
    
    s32 y_start = (s32)ceil(v0.p.y - 0.5f);
    s32 y_end   = (s32)ceil(v1.p.y - 0.5f);
    
    for(s32 y = y_start;
        y <= y_end;
        y++)
    {
        x_start = v2.p.x + (y - y_start)*dxy_left;
        x_end   = v0.p.x + (y - y_start)*dxy_right;
        
        s32 x0 = (s32)ceil(x_start - 0.5f);
        s32 x1 = (s32)ceil(x_end - 0.5f);
        draw_scanline(backbuffer, x0, x1, y, v0,v1,v2);
    }
}

void
draw_triangle(AppBackbuffer *backbuffer, Vertex v0, Vertex v1, Vertex v2)
{
    // NOTE(shvayko): Sort vertices by their Y coordinates
    //                v0 smallest - v2 biggest
    
    if(v0.p.y > v1.p.y)
    {
        Vertex temp = v0;
        v0 = v1;
        v1 = temp;
    }
    
    
    if(v2.p.y < v0.p.y)
    {
        Vertex temp = v2;
        v2 = v0;
        v0 = temp;
    }
    
    
    if(v1.p.y > v2.p.y)
    {
        Vertex temp = v1;
        v1 = v2;
        v2 = temp;
    }
    
    // NOTE(shvayko): Find type of triangle: flat top or flat bottom or general
    
    if(v0.p.y == v1.p.y) // NOTE(shvayko):flat top triangle
    {
        // NOTE(shvayko):Sort vertices by their x coordinates
        if(v0.p.x < v1.p.x)
        {
            Vertex temp = v0;
            v0 = v1;
            v1 = temp;
        }
        draw_flat_top_tri(backbuffer, v0, v2, v1);
    }
    else if(v1.p.y == v2.p.y) // NOTE(shvayko):flat bottom triangle
    {
        if(v2.p.x > v1.p.x)
        {
            Vertex temp = v2;
            v2 = v1;
            v1 = temp;
        }
        draw_flat_bottom_tri(backbuffer, v0, v1, v2);
    }
    else // NOTE(shvayko): General triangle
    {
        // NOTE(shvayko): Find new vertex
        f32 t = (v1.p.y - v0.p.y) / (v2.p.y - v0.p.y);
        v3 new_vertex_p = lerp_v3(v0.p, v2.p, t);
        struct Vertex new_vertex = {new_vertex_p};
        
        // NOTE(shvayko): Decide what major triangle is
        if(new_vertex.p.x > v1.p.x) // NOTE(shvayko): right major
        {
            draw_flat_bottom_tri(backbuffer, v0, new_vertex, v1);
            draw_flat_top_tri(backbuffer,  new_vertex, v2, v1);
        }
        else if(new_vertex.p.x < v1.p.x) // NOTE(shvayko): left major
        {
            draw_flat_bottom_tri(backbuffer, v0,v1,new_vertex);
            draw_flat_top_tri(backbuffer, v1,v2, new_vertex);
        }
    }
}

global bool g_is_init = false;
global Bitmap g_test_bitmap;

Object*
create_triangle_obj(v3 world_p, Vertex v0, Vertex v1, Vertex v2)
{
    Object *result = g_objects + g_object_count;
    Poly poly = {0};
    poly.vertex[0] = v0;
    poly.vertex[1] = v1;
    poly.vertex[2] = v2;
    
    result->world_p = world_p;
    result->polys[result->poly_count] = poly;
    
    result->poly_count = 1;
    result->vertices_count = 3;
    
    g_object_count++;
    return result;
}

Object*
create_cube_obj(v3 world_p, u32 size)
{
    Object *result = g_objects + g_object_count++;
    
    // NOTE(shvayko): cube has a 6 * 2 polygons, 32 vertices
    
    // NOTE(shvayko): The forward face
    Vertex forward_face[6] = 
    {
        {
            {-0.5f, -0.5f, 1.0f},
            {1.0f,0.0f,0.0f},
        },
        {
            {0.5f, 0.5f, 1.0f},
            {1.0f,0.0, 0.0f},
        },
        {
            {-0.5f, 0.5f, 1.0f},
            {1.0f,  0.0f, 0.0f},
        },
        {
            {-0.5f, -0.5f, 1.0f},
            {0.0f,1.0f,0.0f},
        },
        {
            {0.5f, -0.5f, 1.0f},
            {0.0f,1.0f,0.0f},
        },
        {
            {0.5f, 0.5f, 1.0f},
            {0.0f,1.0, 0.0f},
        },
    };
    
    result->world_p = world_p;
    result->poly_count = 0;
    result->vertices_count = 6;
    for(u32 vertex_index = 0;
        vertex_index < 6;
        vertex_index += 3)
    {
        result->vertices[vertex_index + 0] = forward_face[vertex_index + 0].p;
        result->vertices[vertex_index + 1] = forward_face[vertex_index + 1].p;
        result->vertices[vertex_index + 2] = forward_face[vertex_index + 2].p;
        result->polys[result->poly_count].vertex[0] = forward_face[vertex_index + 0];
        result->polys[result->poly_count].vertex[1] = forward_face[vertex_index + 1];
        result->polys[result->poly_count].vertex[2] = forward_face[vertex_index + 2];
        result->poly_count++;
    }
    
    return result;
}

void 
update_and_render(AppBackbuffer *backbuffer, AppMemory *memory)
{
    if(!g_is_init)
    {
        g_test_bitmap = load_bitmap("test_texture.bmp");
#if 0
        struct Vertex v0 = 
        {
            {300.0,100.0f,1.0f},
            {1.0f,0.0f,0.0f},
        };
        
        struct Vertex v1 = 
        {
            {400.0,200.0f,1.0f},
            {1.0f,0.0f,1.0f},
        };
        
        struct Vertex v2 = 
        {
            {300.0,200.0f,1.0f},
            {1.0f,1.0f,0.0f},
        };
        
        g_objects[g_object_count] = *create_triangle_obj(v3f(0.0f,0.0f,0.0f),v0,v1,v2);
#else
        g_objects[g_object_count] = *create_cube_obj(v3f(0.0f,0.0f,0.0f), 0.5f);
#endif
        
        g_is_init = true;
    }
    
    
    m4x4 projection_matrix = build_perspective_projection_matrix(VIEWPORT_WIDTH,VIEWPORT_HEIGHT,90, 1.0f, 10.0f);
    
    // NOTE(shvayko): Test input
    
    
    local_to_world_object(&g_objects[0]);
    
    // NOTE(shvayko): draw test non-textured object
    for(u32 object_index = 0;
        object_index < g_object_count;
        object_index++)
    {
        Object *object = g_objects + object_index;
        for(u32 poly_index = 0;
            poly_index < object->poly_count;
            poly_index++)
        {
            Poly *polygon = object->polys + poly_index;
            
            Vertex v0, v1, v2;
            v0 = polygon->vertex[0];
            v1 = polygon->vertex[1];
            v2 = polygon->vertex[2];
            //NOTE(shvaykO): world space - to - camera space
            
            //NOTE(shvayko): camera space - to - clip space
            
            //NOTE(shvayko): clipping
            
            v4 clip_v0 = v4f(v0.p.x,v0.p.y,v0.p.z,1.0f);
            v4 clip_v1 = v4f(v1.p.x,v1.p.y,v1.p.z,1.0f);
            v4 clip_v2 = v4f(v2.p.x,v2.p.y,v2.p.z,1.0f);
            
            camera_to_clip(projection_matrix, &clip_v0);
            camera_to_clip(projection_matrix, &clip_v1);
            camera_to_clip(projection_matrix, &clip_v2);
            
            //NOTE(shvayko): clip space - to - NDC space
            
            v3 ndc_v0,ndc_v1,ndc_v2;
            perspective_divide(&clip_v0,&clip_v1,&clip_v2,&ndc_v0,&ndc_v1,&ndc_v2);
            
            //NOTE(shvayko): NDC space - to - screen space
            v3 screen_v0,screen_v1,screen_v2;
            viewport(&ndc_v0,&ndc_v1,&ndc_v2,&screen_v0,&screen_v1,&screen_v2);
            
            v0.p = screen_v0;
            v1.p = screen_v1;
            v2.p = screen_v2;
            
            draw_triangle(backbuffer, v0, v1, v2);
        }
    }
}