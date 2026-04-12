#include "Map.h"

// Node of the hash table
struct map_node {
	char* key; 
	Pointer value;    
};

struct map {
	List* arrayOfBuckets;	    // The array of buckets of the hash table (separate chaining)
	int capacity;				// How much space we have allocated so far.
	int size;
	// Functions that will be used by the list in the buckets
	DestroyFunc destroyMapNodes;
	CompareFunc compareMapNodes;
};

// The compare and destroy functions for the Lists of MapNode
Map mapCreate(CompareFunc compare, DestroyFunc destroy, int sizeByFile) {

	Map map = malloc(sizeof(*map));
	//keeping loadfactor low
	map->capacity = sizeByFile * 2 + 7;
	map->size = 0;
	map->arrayOfBuckets = malloc(sizeof(List) * map->capacity);
	map->destroyMapNodes = destroy;
	map->compareMapNodes = compare;

	for (int i = 0; i < map->capacity; i++) {
		map->arrayOfBuckets[i] = NULL;
	}
	return map;
}

void mapInsert(Map map, char* key, Pointer value) {
	// mod to not exceed the size of the array
	int pos = hashFunction(key) % map->capacity; 
	
	// if it is the first node that hashes to this position of the array
	if (map->arrayOfBuckets[pos] == NULL) {
		map->arrayOfBuckets[pos] = listCreate(map->destroyMapNodes, map->compareMapNodes);
	}

	MapNode node = malloc(sizeof(*node));
	node->value = value;
	node->key = key;

	// The list contains MapNode
	listInsert((map->arrayOfBuckets[pos]), node);
	map->size++;
}


Pointer mapFind(Map map, char* key) {
	MapNode node = mapFindNode(map, key);
	if (node == NULL) {
		return NULL;
	} 
	else{
		return node->value;
	}
}

// Returns the node that corresponds to the key, called by mapFind
MapNode mapFindNode(Map map, char* key) {
	int pos = hashFunction(key) % map->capacity;
	
	if (map->arrayOfBuckets[pos] == NULL) {
		return NULL;
	}
	// The work of find will essentially be done by the compare function that we provide each time to the list
		
	ListNode node = listFind(map->arrayOfBuckets[pos], key);
	if (node == NULL) {
		return NULL;
	} 
	else{
		return listNodeValue(node);
	}
}

void mapDestroy(Map map) {
	for (int i = 0; i < map->capacity; i++) {
		if (map->arrayOfBuckets[i] != NULL) {
			listDestroy(map->arrayOfBuckets[i]);
		}
	}
	free(map->arrayOfBuckets);
	free(map);
}

Pointer mapNodeKey(MapNode node) {
	return node->key;
}

Pointer mapNodeValue(MapNode node) {
	return node->value;
}

// djb2 hash function for strings (as mentioned on Piazza, it is allowed to use a ready function)
unsigned int hashFunction(char* value) {
	unsigned int hash = 5381;
	for (char* s = value; *s != '\0'; s++)
		hash = (hash << 5) + hash + *s;
	return hash;
}


int mapGetSize(Map map){
	return map->size;
}

MapNode mapFirst(Map map){
	// Find the first non-empty bucket and get its first node
	for(int i = 0; i < map->capacity; i++){
		if(map->arrayOfBuckets[i] != NULL){
			return listNodeValue( listGetFirst(map->arrayOfBuckets[i]) );
		}
	}
	return NULL;
}

MapNode mapGetNext(Map map, MapNode node){
	//find the bucket of the node and get the next node
	int pos = hashFunction(node->key) % map->capacity;
	List list = map->arrayOfBuckets[pos];
	ListNode listNode = listGetFirst(list);
	while(listNode != NULL){
		//if the node is found, return the next node
		if(listNodeValue(listNode) == node){
			listNode = listGetNext(listNode);
			//if the next node is not null, return it
			if(listNode == NULL){
				for(int i = pos + 1; i < map->capacity; i++){
					if(map->arrayOfBuckets[i] != NULL){
						return listNodeValue( listGetFirst(map->arrayOfBuckets[i]));
					}
				}
				//if there are no more nodes, return NULL
				return NULL;
			}
			//if the next node is not null, return it
			return listNodeValue( listNode);
		}
		listNode = listGetNext(listNode);
	}
	return NULL;
}