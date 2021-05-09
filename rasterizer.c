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
    
    
    f32 dxy_left  = (v2.p.x - v0.p.x) / (v2.p.y - v0.p.y);
    f32 dxy_right = (v1.p.x - v0.p.x) / (v2.p.y - v0.p.y);
    
    f32 x_start = v0.p.x;
    f32 x_end   = v0.p.x;
    
    s32 y_start = (s32)ceil(v0.p.y);
    s32 y_end   = (s32)ceil(v2.p.y);
    
    u32 color = (((s32)(src_color.r*255.0f) << 16) |
                 ((s32)(src_color.g*255.0f) << 8) |
                 ((s32)(src_color.b*255.0f)));
    
    for(s32 y = y_start;
        y <= y_end;
        y++)
    {
        
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
    
    // NOTE(shvayko):CLOCKWISE vertices order 
    
    // NOTE(shvayko): TOP-LEFT FILL CONVENTION 
    //                A top edge, is an edge that is exactly horizontal and is above the             //                other edges.
    //                A left edge, is an edge that is not exactly horizontal and is on the //                left side of the triangle. 
    
    f32 dxy_left  = (v1.p.x - v2.p.x) / (v1.p.y - v0.p.y);
    f32 dxy_right = (v1.p.x - v0.p.x) / (v1.p.y - v0.p.y);
    
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
update_and_render(AppBackbuffer *backbuffer, AppMemory *memory)
{
    struct Vertex v0 = {400.0,100.0f,1.0f};
    struct Vertex v1 = {500.0,200.0f,1.0f};
    struct Vertex v2 = {0.0,200.0f,1.0f};
    draw_flat_bottom_tri(backbuffer,v0,v1,v2,v3f(0.0f,1.0f,0.0f));
    
    struct Vertex v00 = {500.0,300.0f,1.0f};
    struct Vertex v11 = {400.0,400.0f,1.0f};
    struct Vertex v22 = {0.0,300.0f,1.0f};
    draw_flat_top_tri(backbuffer,v00,v11,v22,v3f(0.0f,1.0f,1.0f));
}