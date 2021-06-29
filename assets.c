#define MAX_LINE_LENGTH 64
#define MAX_OBJECT_NAME_LENGTH 32

typedef struct Vertex
{
    v3 p;
    v3 color;
}Vertex;

#if 1
typedef struct Poly
{
    v3 *vertices_list;
    v3 color;
    
    s32 indices[3];
}Poly;
#else
typedef struct Poly
{
    Vertex vertices[3];
}Poly;
#endif


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

typedef struct MeshData
{
    Poly polygons[50968];
    u32 poly_count;
    
    char *object_name;
    
    v3 world_p;
    
    v3 *vertices;
    u32 vertices_count;
    
    f32 *normals;
    u32 normals_count;
    
    f32 *texture_coords;
    
    s32 *indices;
    u32 indices_count;
    
    // TODO(shvayko): Get rid of this restriction
}MeshData;

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

static bool 
is_whitespace(char character)
{
    bool result = (character == ' ');
    return result;
}

static bool
is_digit(char character)
{
    bool result = ((character >= '0') && (character <= '9'));
    return result;
}

int
parse_int(char *string)
{
    s32 result = 0;
    
    s32 sign =  1; // positive
    
    char *scan = string;
    
    // NOTE(shvayko): discard as many whitespace characters (as in isspace) as necessary until the first non-whitespace character is found.
    while(is_whitespace(scan[0]))
    {
        scan++;
    }
    
    if(scan[0] == '-')
    {
        sign = -1; // negative
        scan++;
    }
    
    // TODO(shvayko): Optimize converting. May be tables?
    for(;
        *scan;
        scan++)
    {
        if(scan[0] != '\n')
        {
            if(is_digit(scan[0]))
            {
                s32 shuffle = result * 10;
                s32 num = scan[0] - '0';
                result =  shuffle + num;
            }
        }
    }
    result *= sign; // change the sign
    return result;
}
#if 0
float
parse_float(char *string)
{
    f32 result = 0;
    
    f32 sign = 1.0f; // positive
    
    char *scan = string;
    if(string[0] == '-1')
    {
        sign = -1.0f; // negative
        scan++;
    }
    
    // NOTE(shvayko): Find the point
    s32 point_finded = 0;
    
    for(;
        *scan;
        scan++)
    {
        if(scan[0] != '\n')
        {
            if(scan[0] == '.')
            {
                point_finded = 1;
                continue;
            }
            if(point_finded)
            {
                sign /= 10.0f;
            }
            float shuffle = result * 10;
            int num = scan[0] - '0';
            result =  shuffle + num;
        }
    }
    
    result *= sign;
    
    return result ;
}
#else
static double (*parse_float)(const char * value);
#endif

typedef struct FileContents
{
    char *memory;
    unsigned int size;
}FileContents;

static FileContents
read_entire_file(char *filename)
{
    FileContents result = {0};
    FILE *file = fopen(filename,"rb");
    if(file)
    {
        fseek(file,0,SEEK_END);
        result.size = ftell(file);
        fseek(file,0,SEEK_SET);
        result.memory = (char*)malloc(result.size + 1);
        if(result.memory)
        {
            fread(result.memory,1,result.size,file);
            // null terminate the file
            //result.memory[result.size] = 0;
        }
        else
        {
            free(result.memory);
        }
    }
    else
    {
        ASSERT(!"Error here");
    }
    fclose(file);
    return result;
}

static bool
is_end_of_line(char character)
{
    bool result = (character == '\n') || (character == '\r');
    return result;
}

static void
skip_entire_line(char **at)
{
    for(char *scan = *at;
        *scan;
        scan++)
        
    {
        if(is_end_of_line(*scan))
        {
            break;
        }
        else
        {
            *at = *at + 1;
        }
    }
    
}

static bool
is_end_of_file(char *character)
{
    bool result = (character == "EOF");
    return result;
}

static void
skip_until_next_line(char **at)
{
    while(is_end_of_line(*at[0]))
    {
        *at = *at + 1;
    }
}

MeshData*
load_obj_file(char *filename, v3 world_p)
{
    parse_float = &atof;
    ASSERT(parse_float);
    
    FileContents file = read_entire_file(filename);
    
    char *start = file.memory;
    char *end   = file.memory + file.size;
    
    u32 line_count = 0;
    char *ch = start;
    
    // NOTE(shvayko): Dynamic arrays
    MeshData *mesh = (MeshData*)malloc(sizeof(MeshData));
    mesh->world_p = world_p;
    mesh->vertices = 0;
    mesh->vertices_count = 0;
    mesh->normals = 0;
    mesh->normals_count = 0;
    mesh->texture_coords = 0;
    mesh->indices = 0;
    mesh->indices_count = 0;
    mesh->object_name = malloc(MAX_OBJECT_NAME_LENGTH);
    mesh->poly_count = 0;
    char *object_name = malloc(MAX_OBJECT_NAME_LENGTH);
    
    while(ch != end)
    {
        char *line = (char*)malloc(MAX_LINE_LENGTH);
        memset(line,0,MAX_LINE_LENGTH);
        
        s32 symbols_count = 0;
        while(!is_end_of_line(*ch))
        {
            line[symbols_count++] = *ch++;
        }
        
        // NOTE(shvayko): Parse line
        
        
        char *tmp_line = malloc(strlen(line));
        memcpy(tmp_line,line + 1, strlen(line));
        char *header = strtok(line, " ");
        
        // NOTE(shvayko): if # then skip entire line untill next line
        if(header[0] == '#')
        {
            skip_entire_line(&ch);
        }
        else if(header[0] == 'o')
        {
            // NOTE(shvayko): Object name
            strcpy(mesh->object_name, tmp_line);
        }
        else
        {
            if(header[0] == 'v')
            {
                // NOTE(shvayko): Geometric vertices
                char *v0 = strtok(0, " ");
                char *v1 = strtok(0, " ");
                char *v2 = strtok(0, " ");
                
                f32 parsed_float0  = parse_float(v0);
                // NOTE(shvayko): Reverse Y up to bottom
                f32 parsed_float1  = parse_float(v1) * -1.0f;
                f32 parsed_float2  = parse_float(v2);
                
                v3 vertex = v3f(parsed_float0,parsed_float1,parsed_float2);
                
                ARRAY_PUSH(mesh->vertices, vertex);
                mesh->vertices_count++;
            }
            else if(header[0] == 'vt')
            {
                //NOTE(shvayko):Texture vertices
                char *vt0 = strtok(0, " ");
                char *vt1 = strtok(0, " ");
                char *vt2 = strtok(0, " ");
                
                f32 parsed_float0  = parse_float(vt0);
                f32 parsed_float1  = parse_float(vt1);
                f32 parsed_float2  = parse_float(vt2);
                
                ARRAY_PUSH(mesh->texture_coords, parsed_float0);
                ARRAY_PUSH(mesh->texture_coords, parsed_float1);
                ARRAY_PUSH(mesh->texture_coords, parsed_float2);
            }
            else if(header[0] == 'vn')
            {
                //NOTE(shvayko): Vertex normals
                char *vn0 = strtok(0, " ");
                char *vn1 = strtok(0, " ");
                char *vn2 = strtok(0, " ");
                
                f32 parsed_float0  = parse_float(vn0);
                f32 parsed_float1  = parse_float(vn1);
                f32 parsed_float2  = parse_float(vn2);
                
                ARRAY_PUSH(mesh->normals, parsed_float0);
                ARRAY_PUSH(mesh->normals, parsed_float1);
                ARRAY_PUSH(mesh->normals, parsed_float2);
            }
            else if(header[0] == 'f')
            {
                // NOTE(shvayko): Face information
                // TODO(shvayko): Handle negative indices
                char *value = 0;
                f32 indices_face[64];
                s32 num_face_vertices = 0;
                
                while((value = strtok(0, " ")) != 0)
                {
                    int face_index = parse_int(value) - 1;
                    indices_face[num_face_vertices++] = face_index;
                    ARRAY_PUSH(mesh->indices,face_index);
                    mesh->indices_count++;
                }
                
                
                // NOTE(shvayko):TRIANGULATION STEP
                s32 this_face_indices[128] = {0};
                s32 triangle_count = 0;
                if(num_face_vertices > 3)
                {
                    for(s32  vertices_count = 2;
                        vertices_count < num_face_vertices;
                        vertices_count++)
                    {
                        
                        this_face_indices[triangle_count*3+0]    = indices_face[0]; 
                        this_face_indices[triangle_count*3+1]    = indices_face[vertices_count - 1]; 
                        this_face_indices[triangle_count*3+2]    = indices_face[vertices_count]; 
                        triangle_count++;
                    }
                }
                else
                {
                    this_face_indices[0*3+0]  = indices_face[0]; 
                    this_face_indices[0*3+1]  = indices_face[1]; 
                    this_face_indices[0*3+2]  = indices_face[2]; 
                    triangle_count = 1;
                }
                
                s32 current_index = 0;
                int index = 0;
                for(s32  tri_build_index = 0;
                    tri_build_index < triangle_count;
                    tri_build_index++)
                {
                    
                    index = this_face_indices[current_index++];
                    mesh->polygons[mesh->poly_count].indices[0] = index;
                    v3 vertex0 = mesh->vertices[index];
                    
                    index = this_face_indices[current_index++];
                    mesh->polygons[mesh->poly_count].indices[1] = index;
                    v3 vertex1 = mesh->vertices[index];
                    
                    index = this_face_indices[current_index++];
                    mesh->polygons[mesh->poly_count].indices[2] = index;
                    v3 vertex2 = mesh->vertices[index];
                    
                    mesh->polygons[mesh->poly_count++].vertices_list = mesh->vertices;
                    
                    /*
                                        mesh->polygons[mesh->poly_count].vertices[0].p = vertex0;
                                        
                                        mesh->polygons[mesh->poly_count].vertices[1].p = vertex1;
                                        
                                        mesh->polygons[mesh->poly_count++].vertices[2].p = vertex2;
                    */
                }
            }
            else if(header[0] == 's')
            {
                // NOTE(shvayko): smoothering group
            }
            else
            {
                ASSERT(!"Invalid header!");
            }
        }
        skip_until_next_line(&ch);
        free(line);
        free(tmp_line);
        
    }
    
    return mesh;
}
