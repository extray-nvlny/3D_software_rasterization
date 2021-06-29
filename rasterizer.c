#include "rasterizer.h"
#include "texture.c"
#include "camera.c"
s32 g_viewport_width;
s32 g_viewport_height;

void
clear_backbuffer(AppBackbuffer *backbuffer)
{
    memset(backbuffer->memory,0,backbuffer->width*backbuffer->height*4);
}

void
reset_render_list(RenderList *render_list)
{
    render_list->poly_count = 0;
}

v3
rotate_z(f32 angle, v3 vec)
{
    m4x4 rot_matrix = 
    {
        cosf(angle),sinf(angle),0.0f,0.0f,
        -sinf(angle),cosf(angle),0.0f,0.0f,
        0.0f,0.0f,1.0f,0.0f,
        0.0f,0.0f,0.0f,1.0f,
    };
    
    vec = mul_m4x4v3(rot_matrix,vec);
    return vec;
}

void
local_to_world_object(MeshData *object)
{
    
    for(u32 poly_index = 0;
        poly_index < object->poly_count;
        poly_index++)
    {
        for(u32 vertex_index = 0;
            vertex_index < object->vertices_count;
            vertex_index++)
        {
            v3 *vertex_modify = &object->polygons[poly_index].vertices_list[vertex_index];
            
            m4x4 translate = 
            {
                1,0,0,object->world_p.x,
                0,1,0,object->world_p.y,
                0,0,1,object->world_p.z,
                0,0,0,1
            };
            *vertex_modify = mul_m4x4v3(translate, *vertex_modify);
            int test = 5;
        }
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
    f32 a = 1.0f / (tanf(DEG_TO_RAD(fov*0.5f))*ar);
    f32 b = 1.0f / a;
    f32 d = (-near-far) / (near-far);
    f32 e = (2.0f*far*near) / (near - far);
    
    // NOTE(shvatko): ROW MAJOR
    m4x4 matrix = 
    {
        a,0.0f,0.0f,0.0f,
        0.0f,b,0.0f,0.0f,
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
    screen_v0->x = (ndc_v0->x + 1.0f)*g_viewport_width*0.5f;
    screen_v0->y = (ndc_v0->y + 1.0f)*g_viewport_height*0.5f;
    screen_v0->z = ndc_v0->z;
    
    screen_v1->x = (ndc_v1->x + 1.0f)*g_viewport_width*0.5f;
    screen_v1->y = (ndc_v1->y + 1.0f)*g_viewport_height*0.5f;
    screen_v1->z = ndc_v1->z;
    
    screen_v2->x = (ndc_v2->x + 1.0f)*g_viewport_width*0.5f;
    screen_v2->y = (ndc_v2->y + 1.0f)*g_viewport_height*0.5f;
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
    
    f32 height =  1.0f / (v2.p.y - v0.p.y);
    f32 dxy_left  = (v2.p.x - v0.p.x) * height;
    f32 dxy_right = (v1.p.x - v0.p.x) * height;
    
    f32 x_start = v0.p.x;
    f32 x_end   = v0.p.x;
    
    s32 y_start = (s32)ceil(v0.p.y - 0.5f);
    s32 y_end   = (s32)ceil(v2.p.y - 0.5f);
    
    if(y_start < 0)
    {
        y_start = 0;
    }
    if(y_end > g_viewport_height)
    {
        y_end = g_viewport_height;
    }
    
    
    for(s32 y = y_start;
        y < y_end;
        y++)
    {
        
        x_start = v0.p.x + (y-y_start)*dxy_left;
        x_end   = v0.p.x + (y-y_start)*dxy_right;
        
        if(x_start < 0)
        {
            x_start = 0;
        }
        if(x_end > g_viewport_width)
        {
            x_end = g_viewport_width;
        }
        
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
    
    f32 height =  1.0f / (v1.p.y - v0.p.y);
    f32 dxy_left  = (v1.p.x - v2.p.x) * height;
    f32 dxy_right = (v1.p.x - v0.p.x) * height;
    
    f32 x_start = v2.p.x;
    f32 x_end   = v0.p.x;
    
    s32 y_start = (s32)ceil(v0.p.y - 0.5f);
    s32 y_end   = (s32)ceil(v1.p.y - 0.5f);
    
    if(y_start < 0)
    {
        y_start = 0;
    }
    if(y_end > g_viewport_height)
    {
        y_end = g_viewport_height;
    }
    
    for(s32 y = y_start;
        y < y_end;
        y++)
    {
        x_start = v2.p.x + (y - y_start)*dxy_left;
        x_end   = v0.p.x + (y - y_start)*dxy_right;
        
        if(x_start < 0)
        {
            x_start = 0;
        }
        if(x_end > g_viewport_width)
        {
            x_end = g_viewport_width;
        }
        
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
#if 0
    poly.vertex[0] = v0;
    poly.vertex[1] = v1;
    poly.vertex[2] = v2;
#endif
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
    Object *object = g_objects + g_object_count++;
    
    // NOTE(shvayko): cube has a 6 * 2 polygons, 36 vertices
    object->world_p = world_p;
    object->poly_count = 12;
    object->vertices_count = 36;
    
    // NOTE(shvayko): Vertices array
#if 0
    f32 vertices_buffer[] = 
    {
        // NOTE(shvayko): VERTICES - COLORS
        
        // forward face
        -0.5f, -0.5f, 1.0f, 1.0f,1.0f,0.0f,
        0.5f, 0.5f, 1.0f,   1.0f,0.0f,0.0f,
        -0.5f, 0.5f, 1.0f,  1.0f,0.0f,1.0f,
        
        0.5f, -0.5f, 1.0f,  1.0f,1.0f,0.0f,
        0.5f, 0.5f, 1.0f,   0.0f,1.0f,0.0f,
        -0.5f, -0.5f, 1.0f, 0.0f,1.0f,1.0f,
        
        // right face
        0.5f, -0.5f, 1.0f,  0.0f,1.0f,0.0f,
        0.5f, 0.5f,  2.0f,  0.0f,1.0f,0.0f,
        0.5f, 0.5f,  1.0f,  0.0f,1.0f,0.0f,
        
        0.5f,-0.5f, 2.0f,   0.0f,0.0f,1.0f,
        0.5f, 0.5f, 2.0f,   0.0f,0.0f,1.0f,
        0.5f,-0.5f, 1.0f,   0.0f,0.0f,1.0f,
        
        // back face
        
        -0.5f, -0.5f, 2.0f, 1.0f,0.0f,0.0f,
        0.5f, 0.5f,   2.0f, 1.0f,0.0f,0.0f,
        0.5f, -0.5f,  2.0f, 1.0f,0.0f,0.0f,
        
        -0.5f, -0.5f, 2.0f, 1.0f,0.0f,0.0f,
        -0.5f, 0.5f,  2.0f, 1.0f,0.0f,0.0f,
        0.5f, 0.5f,   2.0f, 1.0f,0.0f,0.0f,
        // left face
        -0.5f, -0.5f, 1.0f, 1.0f,0.2f,1.0f,
        -0.5f, 0.5f,  1.0f, 1.0f,0.2f,1.0f,
        -0.5f, 0.5f,  2.0f, 1.0f,0.2f,1.0f,
        
        -0.5f, -0.5f, 1.0f, 0.0f,0.4f,1.0f,
        -0.5f, 0.5f,  2.0f, 0.0f,0.4f,1.0f,
        -0.5f, -0.5f, 2.0f, 0.0f,0.4f,1.0f,
        
        // top face
        0.5f,-0.5f, 2.0f, 1.0f,0.0f,0.0f,
        0.5f,-0.5f, 1.0f, 1.0f,0.0f,0.0f,
        -0.5f, -0.5f, 1.0f, 0.0f,0.4f,1.0f,
        
        0.5f,-0.5f, 2.0f, 1.0f,0.0f,1.0f,
        -0.5f, -0.5f, 1.0f, 1.0f,0.0f,1.0f,
        -0.5f, -0.5f, 2.0f, 0.0f,0.0f,1.0f,
        
        // bottom face
        
        0.5f,0.5f, 1.0f, 1.0f,1.0f,1.0f,
        0.5f,0.5f, 2.0f, 1.0f,1.0f,1.0f,
        -0.5f,0.5f, 1.0f, 1.0f,1.0f,1.0f,
        
        -0.5f,0.5f, 1.0f, 0.0f,1.0f,0.0f,
        0.5f,0.5f, 2.0f,  0.0f,1.0f,0.0f,
        -0.5f,0.5f, 2.0f, 0.0f,1.0f,0.0f,
        
    };
    
    for(u32 vertex_index = 0;
        vertex_index < object->vertices_count;
        vertex_index++)
    {
        f32 x = vertices_buffer[vertex_index * 6 + 0];
        f32 y = vertices_buffer[vertex_index * 6 + 1];
        f32 z = vertices_buffer[vertex_index * 6 + 2];
        v3 vertex = v3f(x,y,z);
        object->vertices[vertex_index] = vertex;
        
    }
    
    for(u32 poly_index = 0;
        poly_index < object->poly_count;
        poly_index++)
    {
        for(u32 vertex_index = 0;
            vertex_index < 3;
            vertex_index++)
        {
            f32 x = vertices_buffer[(poly_index*18) + vertex_index * 6 + 0];
            f32 y = vertices_buffer[(poly_index*18) + vertex_index * 6 + 1];
            f32 z = vertices_buffer[(poly_index*18) + vertex_index * 6 + 2];
            
            f32 r = vertices_buffer[(poly_index*18) + vertex_index * 6 + 3];
            f32 g = vertices_buffer[(poly_index*18) + vertex_index * 6 + 4];
            f32 b = vertices_buffer[(poly_index*18) + vertex_index * 6 + 5];
            
            v3 vertex = v3f(x,y,z);
            v3 color = v3f(r,g,b);
            object->polys[poly_index].vertices[vertex_index].p = vertex;
            object->polys[poly_index].vertices[vertex_index].color = color;
        }
    }
#else
    
#endif
    return object;
}

MeshData *cube;

void 
update_and_render(AppBackbuffer *backbuffer, AppMemory *memory, Keyboard *input)
{
    if(!g_is_init)
    {
        g_test_bitmap = load_bitmap("test_texture.bmp");
        
        cube = load_obj_file("teapot.obj", v3f(0.0f,0.0f,0.0f));
        g_object_count = 1;
        
        g_viewport_width = backbuffer->width;
        g_viewport_height = backbuffer->height;
        
        g_is_init = true;
    }
    
    clear_backbuffer(backbuffer);
    
    f32 near_plane = 1.0f;
    f32 far_plane  = 10.0f;
    m4x4 projection_matrix = build_perspective_projection_matrix(g_viewport_width,g_viewport_height,90, 
                                                                 near_plane, far_plane);
    
    // NOTE(shvayko): Test input
    v3 dp = v3f(0.0f,0.0f,0.0f);
    cube->world_p = dp;
    if(input->button_right.down)
    {
        dp.x += 0.01f;
    }
    if(input->button_left.down)
    {
        dp.x -= 0.01f;
    }
    if(input->button_up.down)
    {
        dp.z += 0.01f;
    }
    if(input->button_down.down)
    {
        dp.z -= 0.01f;
    }
    cube->world_p = add_v3v3(cube->world_p,dp); 
    
    static f32 angle = 0.0f;
    angle += 0.01f;
    local_to_world_object(cube);
    
    // NOTE(shvayko): draw test non-textured object
    
    v3 colors[12] = 
    {
        v3f(1.0f,0.0f,0.0f),
        v3f(0.0f,1.0f,0.0f),
        v3f(0.0f,0.0f,1.0f),
        
        v3f(1.0f,0.0f,1.0f),
        v3f(1.0f,0.0f,1.0f),
        v3f(0.0f,1.0f,1.0f),
        
        v3f(1.0f,1.0f,1.0f),
        v3f(0.0f,1.0f,1.0f),
        v3f(1.0f,1.0f,0.5f),
        
        v3f(1.0f,0.0f,1.0f),
        v3f(0.0f,0.0f,1.0f),
        v3f(0.0f,1.0f,1.0f),
    };
    
    for(u32 object_index = 0;
        object_index < g_object_count;
        object_index++)
    {
        MeshData *object = cube;
        for(u32 poly_index = 0;
            poly_index < object->poly_count;
            poly_index++)
        {
            Poly *polygon = &object->polygons[poly_index];
            
            s32 index_v0,index_v1,index_v2;
            index_v0 = polygon->indices[0];
            index_v1 = polygon->indices[1];
            index_v2 = polygon->indices[2];
            
            v3 v0 = polygon->vertices_list[index_v0];
            v3 v1 = polygon->vertices_list[index_v1];
            v3 v2 = polygon->vertices_list[index_v2];
            
            v0.z += 8.0f;
            v1.z += 8.0f;
            v2.z += 8.0f;
            
            
            //NOTE(shvaykO): local space - to  world space
            
            // NOTE(shvayko): Backface culling
            // NOTE(shvayko): Left handed system. Clockwise ordering
            bool not_backface = true;
#if 0
            v3 poly_line0 = subtract_v3v3(v0,v2);
            v3 poly_line1 = subtract_v3v3(v1,v2);
            v3 poly_normal = normalize_v3(cross_product_3(poly_line0,poly_line1));
            not_backface = false;
#else
            not_backface = true;
#endif
            //if(dot_product_3(poly_normal,v0.p) > 0.0f)
            if(not_backface)
            {
                //NOTE(shvaykO): world space - to - camera space
                
                //NOTE(shvayko): camera space - to - clip space
                
                //NOTE(shvayko): clipping
                
                v4 clip_v0 = v4f(v0.x,v0.y,v0.z,1.0f);
                v4 clip_v1 = v4f(v1.x,v1.y,v1.z,1.0f);
                v4 clip_v2 = v4f(v2.x,v2.y,v2.z,1.0f);
                
                camera_to_clip(projection_matrix, &clip_v0);
                camera_to_clip(projection_matrix, &clip_v1);
                camera_to_clip(projection_matrix, &clip_v2);
                
                //NOTE(shvayko): clip space - to - NDC space
                
                v3 ndc_v0,ndc_v1,ndc_v2;
                perspective_divide(&clip_v0,&clip_v1,&clip_v2,&ndc_v0,&ndc_v1,&ndc_v2);
                
                //NOTE(shvayko): NDC space - to - screen space
                v3 screen_v0,screen_v1,screen_v2;
                viewport(&ndc_v0,&ndc_v1,&ndc_v2,&screen_v0,&screen_v1,&screen_v2);
                
                v3 color = colors[poly_index % 12];
                Vertex vv0 = {screen_v0,color};
                Vertex vv1 = {screen_v1,color};
                Vertex vv2 = {screen_v2,color};
                
                draw_triangle(backbuffer, vv0, vv1, vv2);
            }
        }
    }
}