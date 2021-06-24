#define MAX_LINE_LENGTH 64
#define MAX_OBJECT_NAME_LENGTH 32

typedef struct Vertex
{
    v3 p;
    v3 color;
}Vertex;

#if 0
typedef struct Poly
{
    v3 *vertices_list;
    v3 color;
    
    s32 vertex[3];
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
    char *object_name;
    
    // TODO(shvayko): Get rid of this restriction
    Poly polygons[1024];
    unsigned int poly_count;
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
    int result = 0;
    
    int sign =  1; // positive
    
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
                int shuffle = result * 10;
                int num = scan[0] - '0';
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
    float result = 0;
    
    float sign = 1.0f; // positive
    
    char *scan = string;
    if(string[0] == '-1')
    {
        sign = -1.0f; // negative
        scan++;
    }
    
    // NOTE(shvayko): Find the point
    int point_finded = 0;
    
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
load_obj_file(char *filename)
{
    parse_float = &atof;
    ASSERT(parse_float);
    
    FileContents file = read_entire_file(filename);
    
    char *start = file.memory;
    char *end   = file.memory + file.size;
    
    unsigned int line_count = 0;
    char *ch = start;
    
    // NOTE(shvayko): Dynamic arrays
    float *vertices_list = 0;
    float *normals = 0;
    float *texture_coords = 0;
    int   *indicies = 0;
    
    float *faces  = 0;
    Poly *polys = 0;
    
    MeshData *mesh = (MeshData*)malloc(sizeof(MeshData));
    
    mesh->object_name = malloc(MAX_OBJECT_NAME_LENGTH);
    
    char *object_name = malloc(MAX_OBJECT_NAME_LENGTH);
    
    while(ch != end)
    {
        char *line = (char*)malloc(MAX_LINE_LENGTH);
        memset(line,0,MAX_LINE_LENGTH);
        
        int symbols_count = 0;
        
        
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
                
                float parsed_float0  = parse_float(v0);
                float parsed_float1  = parse_float(v1);
                float parsed_float2  = parse_float(v2);
                
                ARRAY_PUSH(vertices_list, parsed_float0);
                ARRAY_PUSH(vertices_list, parsed_float1);
                ARRAY_PUSH(vertices_list, parsed_float2);
                
                //printf("Line:[%d] - Vertex %s %s %s\n", line_count++,v0,v1,v2);
            }
            else if(header[0] == 'vt')
            {
                //NOTE(shvayko):Texture vertices
            }
            else if(header[0] == 'vn')
            {
                //NOTE(shvayko): Vertex normals
            }
            else if(header[0] == 'f')
            {
                if(header[0] == 'v')
                {
                    // NOTE(shvayko): Geometric vertices
                    char *v0 = strtok(0, " ");
                    char *v1 = strtok(0, " ");
                    char *v2 = strtok(0, " ");
                    
                    float parsed_float0  = parse_float(v0);
                    float parsed_float1  = parse_float(v1);
                    float parsed_float2  = parse_float(v2);
                    
                    ARRAY_PUSH(vertices_list, parsed_float0);
                    ARRAY_PUSH(vertices_list, parsed_float1);
                    ARRAY_PUSH(vertices_list, parsed_float2);
                    
                    //printf("Line:[%d] - Vertex %s %s %s\n", line_count++,v0,v1,v2);
                }
                else if(header[0] == 'vt')
                {
                    //NOTE(shvayko):Texture vertices
                }
                else if(header[0] == 'vn')
                {
                    //NOTE(shvayko): Vertex normals
                }
                else if(header[0] == 'f')
                {
                    // NOTE(shvayko): Face information
                    // TODO(shvayko): Handle negative indices
                    char *value = 0;
                    float indices_face[64];
                    int num_face_vertices = 0;
                    
                    while((value = strtok(0, " ")) != 0)
                    {
                        indices_face[num_face_vertices++] = parse_int(value);
                    }
                    
                    // 1 5 7 3
                    //          X         Y        Z  
                    // 1 - v 1.000000 1.000000 -1.000000
                    // 5 - v -1.000000 1.000000 -1.000000
                    // 7 - v -1.000000 1.000000 1.000000
                    // 3 - v 1.000000 1.000000 1.000000
                    
                    // NOTE(shvayko):TRIANGULATION STEP
                    int this_face_indices[128] = {0};
                    int triangle_count = 0;
                    if(num_face_vertices > 3)
                    {
                        for(int vertices_count = 2;
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
                    
                    int current_index = 0;
                    for(int tri_build_index = 0;
                        tri_build_index < triangle_count;
                        tri_build_index++)
                    {
                        int index = this_face_indices[current_index++] - 1;
                        float px0 = vertices_list[index * 3 + 0];
                        float py0 = vertices_list[index * 3 + 1];
                        float pz0 = vertices_list[index * 3 + 2];
                        
                        index = this_face_indices[current_index++] - 1;
                        float px1 = vertices_list[index * 3 + 0];
                        float py1 = vertices_list[index * 3 + 1];
                        float pz1 = vertices_list[index * 3 + 2];
                        
                        index = this_face_indices[current_index++] - 1;
                        float px2 = vertices_list[index * 3 + 0];
                        float py2 = vertices_list[index * 3 + 1];
                        float pz2 = vertices_list[index * 3 + 2];
                        
                        mesh->polygons[mesh->poly_count].vertices[0].p.x = px0;
                        mesh->polygons[mesh->poly_count].vertices[0].p.y = py0;
                        mesh->polygons[mesh->poly_count].vertices[0].p.z = pz0;
                        
                        mesh->polygons[mesh->poly_count].vertices[1].p.x = px1;
                        mesh->polygons[mesh->poly_count].vertices[1].p.y = py1;
                        mesh->polygons[mesh->poly_count].vertices[1].p.z = pz1;
                        
                        mesh->polygons[mesh->poly_count].vertices[2].p.x = px2;
                        mesh->polygons[mesh->poly_count].vertices[2].p.y = py2;
                        mesh->polygons[mesh->poly_count++].vertices[2].p.z = pz2;
                    }
                    
                }
                //printf("Line:[%d] - Face %s %s %s\n", line_count++,face0,face1,face2);
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
