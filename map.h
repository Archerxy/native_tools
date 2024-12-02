#ifndef __MAP__
#define __MAP__

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct MapNode_st;
struct Map_st;

typedef struct MapNode_st MapNode;
typedef struct Map_st Map;

typedef void (*map_foreach_cb)(char * key, void *val);
typedef void (*map_key_val_free_cb)(char * key, void *val);


Map * map_new();

void map_free(Map *map, map_key_val_free_cb free_cb);

void map_foreach(Map *map, map_foreach_cb fn);

void* map_put(Map *map, char *key, void *val);

void * map_get(Map *map, char *key);

void * map_remove(Map *map, char *key);

size_t map_size(Map *map);

#endif

