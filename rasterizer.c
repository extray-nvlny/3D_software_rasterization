#include "rasterizer.h"

typedef struct Vertex
{
    v3 p;
}Vertex;

void
draw_scanline(AppBackbuffer *backbuffer,s32 x0,s32 x1,s32 y,u32 color)
{
    u32 *pixel = (u32*)((u8*)backbuffer->memory + x0 * 4 + backbuffer->stride * y);
    
    for(s32 x = x0;
        x < x1;
        x++)
    {
        *pixel++ = color;
    }
}

void
draw_flat_bottom_tri(AppBackbuffer *backbuffer, Vertex v0, Vertex v1, Vertex v2,
                     v3 src_color)
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
    
    s32 y_start = (s32)ceil(v0.p.y);
    s32 y_end   = (s32)ceil(v2.p.y) - 1;
    
    u32 color = (((s32)(src_color.r*255.0f) << 16) |
                 ((s32)(src_color.g*255.0f) << 8) |
                 ((s32)(src_color.b*255.0f)));
    
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
        
        s32 x0 = (s32)ceil(x_start);
        s32 x1 = (s32)ceil(x_end) - 1;
        draw_scanline(backbuffer, x0, x1, y, color);
    }
}

void
draw_flat_top_tri(AppBackbuffer *backbuffer, Vertex v0, Vertex v1, Vertex v2,
                  v3 src_color)
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
    
    s32 y_start = (s32)ceil(v0.p.y);
    s32 y_end   = (s32)ceil(v1.p.y) - 1;
    
    u32 color = (((s32)(src_color.r*255.0f) << 16) |
                 ((s32)(src_color.g*255.0f) << 8) |
                 ((s32)(src_color.b*255.0f)));
    
    for(s32 y = y_start;
        y <= y_end;
        y++)
    {
        x_start = v2.p.x + (y - y_start)*dxy_left;
        x_end   = v0.p.x + (y - y_start)*dxy_right;
        
        s32 x0 = (s32)ceil(x_start);
        s32 x1 = (s32)ceil(x_end) - 1;
        draw_scanline(backbuffer, x0, x1, y, color);
    }
}

void
draw_triangle(AppBackbuffer *backbuffer, Vertex v0, Vertex v1, Vertex v2, v3 color)
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
        draw_flat_top_tri(backbuffer, v0, v2, v1,color);
    }
    else if(v1.p.y == v2.p.y) // NOTE(shvayko):flat bottom triangle
    {
        if(v2.p.x > v1.p.x)
        {
            Vertex temp = v2;
            v2 = v1;
            v1 = temp;
        }
        draw_flat_bottom_tri(backbuffer, v0, v1, v2,color);
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
            draw_flat_bottom_tri(backbuffer, v0, new_vertex, v1 , color);
            draw_flat_top_tri(backbuffer,  new_vertex, v2, v1,    color);
        }
        else if(new_vertex.p.x < v1.p.x) // NOTE(shvayko): left major
        {
            draw_flat_bottom_tri(backbuffer, v0,v1,new_vertex,color);
            draw_flat_top_tri(backbuffer, v1,v2, new_vertex,color);
        }
    }
}

void 
update_and_render(AppBackbuffer *backbuffer, AppMemory *memory)
{
    struct Vertex v0 = {400.0,100.0f,1.0f};
    struct Vertex v1 = {500.0,200.0f,1.0f};
    struct Vertex v2 = {0.0,200.0f,1.0f};
    //draw_triangle(backbuffer,v0,v2,v1,v3f(0.0f,1.0f,0.0f));
    
    struct Vertex v00 = {500.0,300.0f,1.0f};
    struct Vertex v11 = {400.0,400.0f,1.0f};
    struct Vertex v22 = {0.0,300.0f,1.0f};
    //draw_triangle(backbuffer,v22,v11,v00,v3f(0.0f,1.0f,1.0f));
    
    
    struct Vertex v000 = {300.0,200.0f,1.0f};
    struct Vertex v111 = {400.0,500.0f,1.0f};
    struct Vertex v222 = {200.0,400.0f,1.0f};
    draw_triangle(backbuffer,v222,v111,v000,v3f(1.0f,1.0f,1.0f));
    
    
    struct Vertex v0000 = {500.0,200.0f,1.0f};
    struct Vertex v1111 = {400.0,500.0f,1.0f};
    struct Vertex v2222 = {600.0,400.0f,1.0f};
    draw_triangle(backbuffer,v2222,v1111,v0000,v3f(1.0f,1.0f,0.0f));
    
}