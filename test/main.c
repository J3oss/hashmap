#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "../hashmap.h"

#define TESTCOUNT 5000

typedef struct
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
} mystruct_t;

mystruct_t* struct_array;
char** string_array;

void testIntStructMap()
{
    printf("Testing int struct map\n");

    hashmap* map = new_hashmap(uint32_t, mystruct_t, 0);

    for (size_t i = 0; i < TESTCOUNT; i++) {
        hashmap_set(&map, i, struct_array[i]);
    }

    for (size_t i = 0; i < TESTCOUNT; i++) {
        mystruct_t* values = hashmap_get(&map, i);
        printf("for key %lld, value: {%d, %d, %d}\n", i, values[0].a, values[0].b, values[0].c);
        assert(values->a == struct_array[i].a && values->b == struct_array[i].b && values->c == struct_array[i].c);
    }
}

void testStringStructMap()
{
    printf("\nTesting string struct map\n");
    hashmap* map = new_hashmap(char*, mystruct_t, 0);

    for (size_t i = 0; i < TESTCOUNT; i++) {
        hashmap_set(&map, string_array[i], struct_array[i]);  
    }

    for (size_t i = 0; i < TESTCOUNT; i++) {
        mystruct_t* values = hashmap_get(&map, string_array[i]);
        printf("for key %s, value: {%d, %d, %d}\n", string_array[i], values->a, values->b, values->c);
        assert(values->a == struct_array[i].a && values->b == struct_array[i].b && values->c == struct_array[i].c);
    }
}

void testStructStringMap()
{
    printf("\nTesting struct string map\n");
    hashmap* map = new_hashmap(mystruct_t, char*, 0);

    for (size_t i = 0; i < TESTCOUNT; i++) {
        hashmap_set(&map, struct_array[i], string_array[i]);  
    }
    
    for (size_t i = 0; i < TESTCOUNT; i++) {
        char **value = hashmap_get(&map, struct_array[i]);
        printf("for key struct %lld, value: %s\n", i, *value);
        assert(strcmp(*value, string_array[i]) == 0);
    }
}

int main()
{
    struct_array = malloc(sizeof(mystruct_t) * TESTCOUNT);
    for (size_t i = 0; i < TESTCOUNT; i++) 
    {   
        struct_array[i].a = i;
        struct_array[i].b = i*i;
        struct_array[i].c = i*i*i;
    }

    string_array = malloc(sizeof(char*) * TESTCOUNT);
    for (size_t i = 0; i < TESTCOUNT; i++) 
    {   
        string_array[i] = malloc(5);
        string_array[i][0] = 's';
        string_array[i][1] = 't';
        string_array[i][2] = 'r';
        string_array[i][3] = i%255+'0';
        string_array[i][4] = '\0';
    }

    testIntStructMap();
    testStringStructMap();
    testStructStringMap();

    return 0;
}