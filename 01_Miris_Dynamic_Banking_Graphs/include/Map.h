//We implement a Map using a Hash Table with separate chaining
#pragma once  //instruction to the preprocessor to prevent including the same header more than once
#include <stdbool.h>
#include <string.h>
#include "List.h"

typedef struct map* Map;
typedef struct map_node* MapNode;

//O(n) initialize separate chaining lists with NULL
Map mapCreate(CompareFunc compare, DestroyFunc destroy, int sizeByFile);

//O(1)
void mapInsert(Map map, char* key, Pointer value);

//O(1)
void mapRemove(Map map, char* key);

//Returns the value mapped to the given key, or NULL if key does not exist in map. O(1)
Pointer mapFind(Map map, char* key);
MapNode mapFindNode(Map map, char* key);

//O(n)
void mapDestroy(Map map);

//O(1)
Pointer mapNodeKey(Map map, MapNode node);
Pointer mapNodeValue(MapNode node);

unsigned int hashFunction(char* value);