// We implement a Map using a Hash Table with separate chaining
#pragma once  // Directive to the preprocessor to include this header only once
#include <stdbool.h>
#include <string.h>
#include "List.h"
#include "types.h"


// O(n) initialization of the lists for separate chaining with NULL
Map mapCreate(CompareFunc compare, DestroyFunc destroy, int sizeByFile);

// O(1)
void mapInsert(Map map, char* key, Pointer value);

// Returns the value associated with the specific key, or NULL if the key does not exist in the map. O(1)
Pointer mapFind(Map map, char* key);
MapNode mapFindNode(Map map, char* key);

// O(n)
void mapDestroy(Map map);

// O(1)
Pointer mapNodeKey(MapNode node);
Pointer mapNodeValue(MapNode node);

unsigned int hashFunction(char* value);

int mapGetSize(Map map);

MapNode mapFirst(Map map);
MapNode mapGetNext(Map map, MapNode node);