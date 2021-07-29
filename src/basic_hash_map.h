#ifndef __BASIC_MAP__
#define __BASIC_MAP__


#ifdef __cplusplus__
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef size_t
typedef unsigned long size_t;
#endif

typedef struct _bm_key 
{
    uint32_t id;
    void *obj_ptr;
} bm_key_t;

typedef struct _bm_map 
{
    struct _bm_key *map;
    size_t tracked_elements;
    size_t max_elements;
    uint32_t __last_id;
} bm_map_t;

bm_map_t *create_basic_map(size_t max_elems);
void free_basic_map(bm_map_t *map);

bm_key_t *put_basic_map(bm_map_t *map_to_put_in, void *objPtr);
bm_key_t *get_at_basic_map(bm_map_t *map_to_get_from, uint32_t INDEX);
bm_key_t *get_by_id_basic_map(bm_map_t *map_to_get_from, uint32_t id);

#define __BASIC_MAP_IMPL_
#ifdef __BASIC_MAP_IMPL_


bm_map_t *create_basic_map(size_t max_elems)
{
    // Allocate just for the memory map.
    bm_map_t *_new_map = malloc(sizeof(bm_map_t));
    _new_map->map = (struct _bm_key *)malloc(sizeof(bm_key_t) * max_elems);
    _new_map->__last_id = 0;

    memset(_new_map->map, 0, (sizeof(bm_key_t) * max_elems));
    _new_map->max_elements = max_elems;
    _new_map->tracked_elements = 0;
}

void free_basic_map(bm_map_t *map)
{
    if(map != 0)
    {
        if(map->map != 0)
            free(map->map);
        
        free(map);
    }
}

bm_key_t *put_basic_map(bm_map_t* map_to_put_in, void *objPtr)
{
    if(map_to_put_in->tracked_elements + 1 == map_to_put_in->max_elements)
    {
        return NULL; // No space
    }

    ((map_to_put_in->map) + map_to_put_in->tracked_elements)->obj_ptr = objPtr;
    ((map_to_put_in->map) + map_to_put_in->tracked_elements)->id = (map_to_put_in->__last_id) ++;

    map_to_put_in->tracked_elements++;

    return &(map_to_put_in->map[(map_to_put_in->tracked_elements - 1)]);
}

bm_key_t *get_at_basic_map(bm_map_t* map_to_get_from, uint32_t INDEX)
{
    if(map_to_get_from != NULL 
        && INDEX < map_to_get_from->tracked_elements)
    {
        return &(map_to_get_from->map[INDEX]);
    }

    return NULL;
}

bm_key_t *get_by_id_basic_map(bm_map_t* map_to_get_from, uint32_t id)
{
    if(map_to_get_from != NULL
        && map_to_get_from->tracked_elements > 0)
    {
        for(int i = 0; i < map_to_get_from->tracked_elements; i++)
        {
            if(map_to_get_from->map[i].id == id)
                return &(map_to_get_from->map[i]);
        }
    }

    return NULL;
}
#endif


#ifdef __cplusplus__
}
#endif

#endif // __BASIC_MAP__