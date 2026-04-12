#include "builderUtils.h"
#include <stdlib.h>
#include <string.h>
#include "Map.h"


int builderCompareWords(Pointer a, Pointer b){
	MapNode nodeB = (MapNode)b;
	return strcmp(a, mapNodeKey(nodeB));
}



void destroyMapNode(Pointer node){
	MapNode mapNode = (MapNode)node;
	free(mapNodeKey(mapNode));
	free(mapNodeValue(mapNode));
	free(mapNode);
}
