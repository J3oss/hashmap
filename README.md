# Hashmap

A closed-addressing hashmap implementation in C with plans to support cuckoo hashing.

## Features

- Supports arbitrary key-value types
- Custom comparison function for keys

## Installation

### Dependencies
- A C compiler (GCC, Clang, or MSVC)
- [vector](https://github.com/J3oss/vector)

## Usage

```c
#include "hashmap.h"

typedef struct
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
} mystruct_t;
mystruct_t s = {10, 20, 30};

int main()
{
    hashmap* map = new_hashmap(mystruct_t, float, my_compare_function);

    hashmap_set(&map, s, 2.0);
    void* value = hashmap_get(&map, s);
    hashmap_remove(&map, s);
    
    hashmap_free(&map);

    return 0;
}
```