typedef struct Bitmap
{
    void *memory;
    u32 width;
    u32 height;
    u32 stride;
}Bitmap;


#pragma pack(push,1)
typedef struct BitmapFormat
{
    u16 file_type;
    u32 file_size;
    u16 reserved1;
    u16 reserved2;
    u32 bitmap_offset;
    
    u32 size;           
    s32 width;          
    s32 height;         
	u16 planes;         
	u16 bits_per_pixel;   
	u32 compression;    
	u32 size_of_bitmap;   
    s32 horz_resolution; 
    s32 vert_resolution; 
	u32 colors_used;     
	u32 colors_important;
    
    u32 red_mask;  
    u32 green_mask;
    u32 blue_mask; 
    u32 alpha_mask;
}BitmapFormat;
#pragma pack(pop)

Bitmap
load_bitmap(char *filename)
{
    Bitmap result = {0};
    
    FileContent file_content = win_read_file(filename);
    if(file_content.memory)
    {
        BitmapFormat *bitmap_format = (BitmapFormat*)file_content.memory;
        
        result.memory = (u8*)file_content.memory + bitmap_format->bitmap_offset;
        result.width  = bitmap_format->width;
        result.height = bitmap_format->height;
        result.stride = result.width * 4;
    }
    else
    {
        // TODO(shvayko): Logging
    }
    
    return result;
}