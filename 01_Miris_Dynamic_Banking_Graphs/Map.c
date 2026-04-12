#include "Map.h"
#include "Graph.h"

//Hash table node
struct map_node{
	char* key; 
	//Pointer to a graph node GraphNode = struct graph_node*.
	//This gives fast graph access while preserving structure independence as required.
	GraphNode value;    
};

struct map {
	List* arrayOfBuckets;	    //Hash table bucket array (separate chaining)
	int capacity;				//Currently allocated capacity
	//Functions used by bucket lists
	DestroyFunc destroyMapNodes;
	CompareFunc compareMapNodes;
};


//Compare and destroy for lists of MapNode
Map mapCreate(CompareFunc compare, DestroyFunc destroy, int sizeByFile) {

	Map map = malloc(sizeof(*map));
	map->capacity = sizeByFile * 3 + 7;
	map->arrayOfBuckets = malloc(sizeof(List) * map->capacity);
	map->destroyMapNodes = destroy;
	map->compareMapNodes = compare;

	for (int i = 0; i < map->capacity; i++){
		map->arrayOfBuckets[i] = NULL;
	}
	return map;
}


void mapInsert(Map map, char* key, Pointer value) {
	int pos = hashFunction(key) % map->capacity; //mod to stay within array bounds
	
	//If this is the first node hashed into this array position
	if(map->arrayOfBuckets[pos] == NULL){
		map->arrayOfBuckets[pos] = listCreate(map->destroyMapNodes, map->compareMapNodes);
	}

	MapNode node = malloc(sizeof(*node));
	node->value = value;
	node->key = key;

	//The list stores MapNode
	listInsert((map->arrayOfBuckets[pos]), node);
}


void mapRemove(Map map, char* key) {
    int pos = hashFunction(key) % map->capacity;
    if (map->arrayOfBuckets[pos] == NULL) {
        return;
    }
    listDeleteNode(map->arrayOfBuckets[pos], key);
}

Pointer mapFind(Map map, char* key) {
	MapNode node = mapFindNode(map, key);
	if(node == NULL){
		return NULL;
	}
	else{
		return node->value;
	}
}

//Returns the node that corresponds to key, called by mapFind
MapNode mapFindNode(Map map, char* key) {
	int pos = hashFunction(key) % map->capacity;
	
	if( map->arrayOfBuckets[pos] == NULL){
		return NULL;
	}
	//find logic is essentially handled by compare passed to list
	ListNode node = listFind(map->arrayOfBuckets[pos], key);
	if(node == NULL){
		return NULL;
	}
	else{
		return listNodeValue(node);
	}
}

//We only clear pointers because destroyGraph already freed node memory
void mapDestroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		if(map->arrayOfBuckets[i] != NULL){
			listDestroy(map->arrayOfBuckets[i]);
		}
	}
	free(map->arrayOfBuckets);
	free(map);
}


Pointer mapNodeKey(Map map, MapNode node) {
	return node->key;
}

Pointer mapNodeValue(MapNode node) {
	return node->value;
}

//djb2 hash function for strings (as mentioned on Piazza, using a ready function is allowed)
unsigned int hashFunction(char* value) {
    unsigned int  hash = 5381;
    for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;
    return hash;
}