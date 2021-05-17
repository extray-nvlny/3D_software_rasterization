#include "rasterizer.h"
#include "assets.c"

typedef struct Vertex
{
    v3 p;
    v3 color;
}Vertex;

typedef struct TexVertex
{
    v3 p;
    v2 uv;
}TexVertex;

void
draw_flat_top_textured_tri(AppBackbuffer *backbuffer,
                           TexVertex v0, TexVertex v1, TexVertex v2,
                           Bitmap *texture)
{
    f32 height = 1.0f / (v1.p.y - v0.p.y);
    f32 dxdy_left = (v1.p.x - v2.p.x) * height;
    f32 dudy_left = (v1.uv.u - v2.uv.u) * height;
    f32 dvdy_left = (v1.uv.v - v2.uv.v) * height;
    
    f32 dxdy_right = (v1.p.x - v0.p.x) * height;
    f32 dudy_right = (v1.uv.u - v0.uv.u) * height;
    f32 dvdy_right = (v1.uv.v - v0.uv.v) * height;
    
    
    // NOTE(shvayko): Starting points
    f32 x_left  = v2.p.x;
    f32 u_left  = v2.uv.u;
    f32 v_left  = v2.uv.v;
    
    f32 x_right = v0.p.x;
    f32 u_right = v0.uv.u;
    f32 v_right = v0.uv.v;
    
    s32 y_start = ceil(v0.p.y);
    s32 y_end   = ceil(v1.p.y);
    
    for(s32 y = y_start;
        y < y_end;
        y++)
    {
        s32 x0 = (s32)ceil(x_left);
        s32 x1 = (s32)ceil(x_right);
        
        
        f32 dx = 1.0 / (x0 - x1);
        
        
        f32 du = (u_left - u_right) * dx;
        f32 dv = (v_left - v_right) * dx;
        
        
        f32 start_u = u_left;
        f32 start_v = v_left;
        
        for(s32 x = x0;
            x < x1;
            x++)
        {
            u32 *dst_pixel = (u32*)((u8*)backbuffer->memory + x * 4 + y * backbuffer->stride);
            
            s32 texel_x = (s32)(start_u * (texture->width - 1));
            s32 texel_y = (s32)(start_v * (texture->height - 1));
            
            
            u32 *src_pixel = (u32*)((u8*)texture->memory + texel_x  * 4 + 
                                    texel_y * texture->stride);
            
            *dst_pixel++ = *src_pixel;
            
            
            start_u += du;
            start_v += dv;
        }
        
        x_left += dxdy_left;
        u_left += dudy_left;
        v_left += dvdy_left;
        
        x_right += dxdy_right;
        u_right += dudy_right;
        v_right += dvdy_right;
    }
}

void
draw_flat_bottom_textured_tri(AppBackbuffer *backbuffer,
                              TexVertex v0, TexVertex v1, TexVertex v2,
                              Bitmap *texture)
{
    f32 height = 1.0f / (v2.p.y - v0.p.y);
    f32 dxdy_left  = (v2.p.x  - v0.p.x)   * height;
    f32 dudy_left  = (v2.uv.u - v0.uv.u) * height;
    f32 dvdy_left  = (v2.uv.v - v0.uv.v) * height;
    
    f32 dxdy_right = (v1.p.x  - v0.p.x) * height;
    f32 dudy_right = (v1.uv.u - v0.uv.u) * height;
    f32 dvdy_right = (v1.uv.v - v0.uv.v) * height;
    
    // NOTE(shvayko): Starting points
    f32 x_left  = v0.p.x;
    f32 u_left  = v0.uv.u;
    f32 v_left  = v0.uv.v;
    
    f32 x_right = v0.p.x;
    f32 u_right = v0.uv.u;
    f32 v_right = v0.uv.v;
    
    s32 y_start = ceil(v0.p.y);
    s32 y_end   = ceil(v2.p.y);
    
    for(s32 y = y_start;
        y < y_end;
        y++)
    {
        s32 x0 = (s32)ceil(x_left);
        s32 x1 = (s32)ceil(x_right);
        
        
        f32 dx = 1.0f / (x0 - x1);
        
        
        f32 du = (u_left-u_right) * dx;
        f32 dv = (v_left-v_right) * dx;
        
        f32 start_u = u_left;
        f32 start_v = v_left;
        
        for(s32 x = x0;
            x < x1;
            x++)
        {
            u32 *dst_pixel = (u32*)((u8*)backbuffer->memory + x * 4 + y * backbuffer->stride);
            s32 texel_x = (s32)(start_u * (texture->width - 1.0f));
            s32 texel_y = (s32)(start_v * (texture->height - 1.0f));
            
            u32 *src_pixel = (u32*)((u8*)texture->memory + texel_x  * 4 + 
                                    texel_y * texture->stride);
            
            *dst_pixel++ = *src_pixel;
            
            start_u += du;
            start_v += dv;
        }
        
        x_left += dxdy_left;
        u_left += dudy_left;
        v_left += dvdy_left;
        
        x_right += dxdy_right;
        u_right += dudy_right;
        v_right += dvdy_right;
    }
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

void 
update_and_render(AppBackbuffer *backbuffer, AppMemory *memory)
{
    if(!g_is_init)
    {
        g_test_bitmap = load_bitmap("test_texture.bmp");
        
        g_is_init = true;
    }
    
    struct TexVertex tv0 = 
    {
        {300.0,100.0f,1.0f},
        {0.0f,1.0f,}
    };
    
    
    struct TexVertex tv1 = 
    {
        {400.0,200.0f,1.0f},
        {1.0f,0.0f,}
    };
    
    
    struct TexVertex tv2 = 
    {
        {300.0,200.0f,1.0f},
        {0.0f,0.0f}
    };
    
    draw_flat_bottom_textured_tri(backbuffer, tv0, tv1, tv2, &g_test_bitmap);
    
    struct TexVertex tv00 = 
    {
        {400.0,100.0f,1.0f},
        {1.0f,1.0f,}
    };
    
    
    struct TexVertex tv11 = 
    {
        {400.0,200.0f,1.0f},
        {1.0f,0.0f,}
    };
    
    
    struct TexVertex tv22 = 
    {
        {300.0,100.0f,1.0f},
        {0.0f,1.0f}
    };
    
    draw_flat_top_textured_tri(backbuffer, tv00, tv11, tv22, &g_test_bitmap);
    
}