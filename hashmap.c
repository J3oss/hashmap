
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "MurmurHash3.h"
#include "hashmap.h"
#include "thirdparty/vector/vec.h"

#ifdef _MSC_VER
#  include <intrin.h>
#  define __builtin_popcount __popcnt
#endif

#define KEYS(X) ((*(X))->keys)
#define SEED(X) ((*(X))->seed)
#define VALUES(X) ((*(X))->values)
#define KEY_SIZE(X) ((*(X))->key_size)
#define VALUE_SIZE(X) ((*(X))->value_size)
#define BUCKET_COUNT(X) ((*(X))->bucket_count)
#define ENTRY_COUNT(X) ((*(X))->entry_count)
#define LOAD_FACTOR(X) ((*(X))->load_factor)
#define GET_PTR(IN, SIZE_IN) ( __builtin_popcount((SIZE_IN)) == 1 && (SIZE_IN) <= 8) ? &(IN):(void*)(IN)

#define GROWTH_RATE 2

void rehash(hashmap** map);
void resize(hashmap** old_map);
void CALLCONVENTION _impl_hashmap_set(hashmap **pp_map, void* key, void* value);

typedef struct
{
    uint64_t h1;
    uint64_t h2;
} hash_t;

hash_t hash(const void* key, const size_t keySize, const size_t seed)
{
    hash_t h;
    MurmurHash3_x64_128(key, keySize, seed, &h);

    return h;
}

int defaultCompare(const void *a, const void *b, const void* data) 
{
    const size_t keySize = *(size_t*)data;
    return memcmp(a, b, keySize);
}

hashmap* _impl_new_hashmap(size_t bucket_size, uint32_t kSize, uint32_t vSize, int (*compare)(const void *a, const void *b, const void* data)) 
{
    hashmap* h = malloc(sizeof(hashmap));

    h->bucket_count = bucket_size ? bucket_size:0;
    h->key_size = kSize;
    h->value_size = vSize;
    h->load_factor = 5;
    h->entry_count = 0;

    h->keys = calloc(bucket_size, sizeof(void*));
    h->values = calloc(bucket_size, sizeof(void*));
    for (size_t i=0; i < bucket_size; i++) 
    {
        h->keys[i] = _impl_new_vec(kSize, 0);
        h->values[i] = _impl_new_vec(vSize, 0);
    }

    h->compare = defaultCompare;
    return h;
}

void CALLCONVENTION _impl_hashmap_set(hashmap **pp_map, void* key, void* value)
{
    size_t key_size = KEY_SIZE(pp_map);
    size_t bucket_count = BUCKET_COUNT(pp_map);

    hash_t hashes = hash(key, key_size, SEED(pp_map));
    uint64_t bucket_id = hashes.h1%bucket_count;

    vec_push_ptr(&KEYS(pp_map)[bucket_id], key);
    vec_push_ptr(&VALUES(pp_map)[bucket_id], value);

    (*pp_map)->entry_count++;
    if ((*pp_map)->entry_count > (*pp_map)->bucket_count * (*pp_map)->load_factor) 
    {
        resize(pp_map);
    }
}
void CALLCONVENTION hashmap_set(hashmap **pp_map, ...)
{
    VA_LIST args;
    VA_START(args, pp_map);

    size_t key_size = KEY_SIZE(pp_map);
    size_t value_size = VALUE_SIZE(pp_map);

    size_t key = VA_ARG(args, size_t);
    size_t value = VA_ARG(args, size_t);

    char* ptr_key = GET_PTR(key, key_size);
    char* ptr_value = GET_PTR(value, value_size);
    if (hashmap_contains(pp_map, key)) 
    {
        return;
    }

    return _impl_hashmap_set(pp_map, ptr_key, ptr_value);
}

void* CALLCONVENTION hashmap_get(hashmap **pp_map, ...)
{
    VA_LIST args;
    VA_START(args, pp_map);
    
    size_t key_size = KEY_SIZE(pp_map);
    size_t value_size = VALUE_SIZE(pp_map);
    size_t bucket_count = BUCKET_COUNT(pp_map);
    
    size_t key = VA_ARG(args, size_t);
    void* ptr_key = GET_PTR(key, key_size);

    hash_t hashes = hash(ptr_key, key_size, SEED(pp_map));
    uint64_t bucket_id = hashes.h1%bucket_count;

    char* keys = KEYS(pp_map)[bucket_id];
    char* values = VALUES(pp_map)[bucket_id];
    for (size_t i = 0; i < vec_size(&keys); i++) 
    {
        if (memcmp(keys + i * key_size, ptr_key, key_size) == 0) 
        {
            return values + i * value_size;
        }
    }

    return 0;
}

size_t CALLCONVENTION hashmap_contains(hashmap **pp_map, ...)
{
    VA_LIST args;
    VA_START(args, pp_map);
    
    size_t key_size = KEY_SIZE(pp_map);
    size_t bucket_count = BUCKET_COUNT(pp_map);

    size_t key = VA_ARG(args, size_t);
    void* ptr_key = GET_PTR(key, key_size);

    hash_t hashes = hash(ptr_key, key_size, SEED(pp_map));
    uint64_t bucket_id = hashes.h1%bucket_count;

    char* keys = KEYS(pp_map)[bucket_id];
    for (size_t i = 0; i < vec_size(&keys); i++) 
    {
        if (memcmp(keys + i * key_size, ptr_key, key_size) == 0) 
        {
            return 1;
        }
    }

    return 0;
}

void CALLCONVENTION hashmap_remove(hashmap **pp_map, ...)
{
    VA_LIST args;
    VA_START(args, pp_map);
    
    size_t key_size = KEY_SIZE(pp_map);
    size_t value_size = VALUE_SIZE(pp_map);
    size_t bucket_count = BUCKET_COUNT(pp_map);

    size_t key = VA_ARG(args, size_t);
    void* ptr_key = GET_PTR(key, key_size);

    hash_t hashes = hash(ptr_key, key_size, SEED(pp_map));
    uint64_t bucket_id = hashes.h1%bucket_count;

    char* keys = KEYS(pp_map)[bucket_id];
    char* values = VALUES(pp_map)[bucket_id];
    for (size_t i = 0; i < vec_size(&keys); i++) 
    {
        if (memcmp(keys + i * key_size, ptr_key, key_size) == 0) 
        {
            vec_remove(&keys, i);
            vec_remove(&values, i);
            break;
        }
    }
}

void resize(hashmap** pp_old_map)
{
    size_t key_size = KEY_SIZE(pp_old_map);
    size_t value_size = VALUE_SIZE(pp_old_map);

    size_t old_bucket_count = BUCKET_COUNT(pp_old_map);
    size_t new_bucket_count = GROWTH_RATE * old_bucket_count;

    hashmap* new_map = _impl_new_hashmap(new_bucket_count, key_size, value_size, (*pp_old_map)->compare);

    for (size_t i=0; i < old_bucket_count; i++) 
    {   
        char* keys = KEYS(pp_old_map)[i];
        char* values = VALUES(pp_old_map)[i];

        size_t bucket_size = vec_size(&keys);
        for (size_t j=0; j < bucket_size; j++) 
        {
            _impl_hashmap_set(&new_map, keys+j*key_size, values+j*value_size);
        }
    }

    hashmap_free(pp_old_map);
    *pp_old_map = new_map;
}

void hashmap_free(hashmap** pp_map)
{
    size_t bucket_count = BUCKET_COUNT(pp_map);
    for (size_t i=0; i < bucket_count; i++) 
    {   
        vec_free(&KEYS(pp_map)[i]);
        vec_free(&VALUES(pp_map)[i]);
    }
    free(*pp_map);
}

void rehash(hashmap** map)
{

}
