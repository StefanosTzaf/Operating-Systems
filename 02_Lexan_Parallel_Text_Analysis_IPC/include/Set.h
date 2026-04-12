#include <stdbool.h>
#include "types.h"

Set setCreate(CompareFunc compare, DestroyFunc destroyValue);

int setSize(Set set);

void setInsert(Set set, Pointer value);
SetNode setInsertNode(Set set, Pointer value, SetNode subtreeRoot);

void nodeDestroy(SetNode node, DestroyFunc destroyValue);
void setDestroy(Set set);

SetNode setFirst(Set set);
SetNode setLast(Set set);

SetNode setMin(SetNode node);
SetNode setMax(SetNode node);

Pointer setNodeValue(Set set, SetNode node);

SetNode nodeFindPrevious(SetNode root, Set set, SetNode currentNode);
SetNode nodeFindMax(SetNode node);

SetNode getRootNode(Set set);