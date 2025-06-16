#ifndef CL_MAP_H_
#define CL_MAP_H_

#include <stdint.h>
#include <stddef.h>

#define MAP_BUCKET_SIZE 512

typedef struct MapNode MapNode;

struct MapNode {
    void *data;
    size_t data_size;

    char *key;

    MapNode *next;
};

typedef struct {
    MapNode* nodes[MAP_BUCKET_SIZE]; // TODO: increase memory space as needed
    size_t length;
} Map;

Map *map_new(void);
void map_set(Map *map, char *key, void *data, size_t data_size);
void map_set_i(Map *map, char *key, int i);
void map_set_s(Map *map, char *key, char *s);
void *map_get(Map *map, char *key);
void map_free(Map *map);

#endif // CL_MAP_H_
