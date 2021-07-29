/* date = May 4th 2021 4:34 pm */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float    f32;
typedef double   f64;

#define global static
#define local_persist static
#define internal static

#define ASSERT(expr) if(!(expr)) { *(u32*)0 = 0;}
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#define KILOBYTES(x) x*1024
#define MEGABYTES(x) KILOBYTES(x)*1024 
#define GIGABYTES(x) MEGABYTES(x)*1024
#define TERABYTES(x) GIGABYTES(x)*1024

typedef struct
{
    bool down;
    bool pressed;
    bool released;
}DigitalButton;

typedef struct Keyboard
{
    union
    {
        DigitalButton e[4];
        struct
        {
            DigitalButton button_up;
            DigitalButton button_down;
            DigitalButton button_left;
            DigitalButton button_right;
        };
    };
}Keyboard;

typedef struct
{
    void *memory;
    s32 width;
    s32 height;
    s32 stride;
}AppBackbuffer;

enum
{
    DebugCycleCount_update_and_render = 0,
    DebugCycleCount_pipeline,
    DebugCycleCount_load_model,
    DebugCycleCount_draw_triangle,
    DebugCycleCount_draw_flat_top_tri,
    DebugCycleCount_draw_flat_bottom_tri,
    DebugCycleCount_draw_scanline,
    
    DebugCycleCount_count
};

typedef struct DebugCounter
{
    u64 cycles;
    u64 was_called;
}DebugCounter;

typedef struct
{
    s64  permanent_storage_size;
    s64  transient_storage_size;
    void *permanent_storage;
    void *transient_storage;
    
    bool is_memory_init;
    
    DebugCounter debug_counters[10];
}AppMemory;

extern AppMemory *g_debug_memory;

#define START_TIMED_BLOCK(name) u64 start_cycle_count_##name = __rdtsc();
#define END_TIMED_BLOCK(name) g_debug_memory->debug_counters[DebugCycleCount_##name].cycles += __rdtsc() - start_cycle_count_##name;g_debug_memory->debug_counters[DebugCycleCount_##name].was_called++;

void *memset(void *, int,size_t);


typedef struct FileContent
{
    void *memory;
    u32 file_size;
}FileContent;

FileContent win_read_file(char *filename);
<<<<<<< HEAD
void win_write_file(char *filename, void *data, u32 bytes_to_write);
=======
>>>>>>> c8f0ac7a68ed4f6288d6d09b7908b62e68f25d95

// NOTE(shvayko): Stretchy buffers implementation
typedef struct ArrayHeader
{
    size_t length;
    size_t capacity;
    char buf[0];
}ArrayHeader;

#define ARRAY_LENGTH(a) ((a) ? ARRAY_HEADER(a)->length : 0)
#define ARRAY_CAPACITY(a) ((a) ? ARRAY_HEADER(a)->capacity : 0)

#define ARRAY_HEADER(a) ((ArrayHeader*)((char*)a - offsetof(ArrayHeader,buf)))
#define ARRAY_FITS(a,item) ((ARRAY_LENGTH(a) + item) <= ARRAY_CAPACITY(a))
#define ARRAY_FIT(a,item) ((ARRAY_FITS(a,item) ? 0 : (a = ARRAY_GROW(a,ARRAY_LENGTH(a) + item,sizeof(*a)))))

#define ARRAY_PUSH(a,item) ((ARRAY_FIT(a,1), a[ARRAY_HEADER(a)->length++] = item))
#define ARRAY_GROW(a, length,elem_size) (array_grow_(a, length,elem_size))
#define ARRAY_FREE(a) (a ? free(ARRAY_HEADER(a)) : 0)

void *
array_grow_(const void *array, size_t new_len, size_t elem_size)
{
    size_t new_cap = 1+2*ARRAY_CAPACITY(array);
    ASSERT(new_cap >= new_len);
    size_t new_size = offsetof(ArrayHeader,buf) + new_cap * elem_size;
    ArrayHeader *new_header;
    if(array)
    {
        new_header = realloc(ARRAY_HEADER(array),new_size);
    }
    else
    {
        new_header = malloc(new_size);
        new_header->length = 0;
    }
    new_header->capacity = new_cap;
    return new_header->buf;
}



#endif //PLATFORM_H
