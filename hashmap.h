#include <stdint.h>

#ifndef _WIN32
#define CALLCONVENTION __attribute__((ms_abi))
#define VA_LIST __builtin_ms_va_list
#define VA_START __builtin_ms_va_start
#define VA_ARG __builtin_va_arg
#else
#define CALLCONVENTION
#define VA_LIST va_list
#define VA_START va_start
#define VA_ARG va_arg
#endif

typedef struct 
{
    float load_factor;
    size_t seed;

    size_t entry_count;
    size_t bucket_count;

    size_t key_size;
    void** keys;
    
    size_t value_size;
    void** values;

    int (*compare)(const void *a, const void *b, const void* data);
} hashmap;

#define new_hashmap(kType, vType, cp) _impl_new_hashmap(10, sizeof(kType), sizeof(vType), cp)
hashmap* _impl_new_hashmap(size_t bucket_size, uint32_t kSize, uint32_t vSize, int (*compare)(const void *a, const void *b, const void* data));

void  CALLCONVENTION hashmap_set(hashmap **map, ...);
void* CALLCONVENTION hashmap_get(hashmap **map, ...);
size_t  CALLCONVENTION hashmap_contains(hashmap **map, ...);
void  CALLCONVENTION hashmap_remove(hashmap **map, ...);
void hashmap_free(hashmap** map);