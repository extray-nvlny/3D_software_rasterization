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

void
draw_tex_tri(AppBackbuffer *backbuffer, TexVertex v0, TexVertex v1, TexVertex v2, Bitmap *texture)
{
    // NOTE(shvayko): Sort vertices by their Y coordinates
    //                v0 smallest - v2 biggest
    
    if(v0.p.y > v1.p.y)
    {
        TexVertex temp = v0;
        v0 = v1;
        v1 = temp;
    }
    
    
    if(v2.p.y < v0.p.y)
    {
        TexVertex temp = v2;
        v2 = v0;
        v0 = temp;
    }
    
    
    if(v1.p.y > v2.p.y)
    {
        TexVertex temp = v1;
        v1 = v2;
        v2 = temp;
    }
    
    // NOTE(shvayko): Find type of triangle: flat top or flat bottom or general
    
    if(v0.p.y == v1.p.y) // NOTE(shvayko):flat top triangle
    {
        // NOTE(shvayko):Sort vertices by their x coordinates
        if(v0.p.x < v1.p.x)
        {
            TexVertex temp = v0;
            v0 = v1;
            v1 = temp;
        }
        draw_flat_top_textured_tri(backbuffer, v0, v2, v1, texture);
    }
    else if(v1.p.y == v2.p.y) // NOTE(shvayko):flat bottom triangle
    {
        if(v2.p.x > v1.p.x)
        {
            TexVertex temp = v2;
            v2 = v1;
            v1 = temp;
        }
        draw_flat_bottom_textured_tri(backbuffer, v0, v1, v2,texture);
    }
    else // NOTE(shvayko): General triangle
    {
        // NOTE(shvayko): Find new vertex
        f32 t = (v1.p.y - v0.p.y) / (v2.p.y - v0.p.y);
        v3 new_vertex_p = lerp_v3(v0.p, v2.p, t);
        // TODO(shvayko): U V  interpolation
        struct TexVertex new_vertex = {new_vertex_p};
        
        // NOTE(shvayko): Decide what major triangle is
        if(new_vertex.p.x > v1.p.x) // NOTE(shvayko): right major
        {
            draw_flat_bottom_textured_tri(backbuffer, v0, new_vertex, v1,texture);
            draw_flat_top_textured_tri(backbuffer,  new_vertex, v2, v1,texture);
        }
        else if(new_vertex.p.x < v1.p.x) // NOTE(shvayko): left major
        {
            draw_flat_bottom_textured_tri(backbuffer, v0,v1,new_vertex,texture);
            draw_flat_top_textured_tri(backbuffer, v1,v2, new_vertex,texture);
        }
    }
}
