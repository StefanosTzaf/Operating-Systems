//--------------------------------------Implementation of a doubly linked list-----------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include "types.h"


// (O(1))
List listCreate(DestroyFunc destroyValue, CompareFunc compare);

// (O(1))
void listInsert(List list, Pointer value);

// (O(n)) due to ListFind
void listDeleteNode(List list, Pointer value);
// O(1)
void listRemoveLast(List list);

// (O(n))
void listDestroy(List list);

// (O(n))
ListNode listFind(List list, Pointer value);

// (O(1))
ListNode listGetLast(List list);
ListNode listGetFirst(List list);
ListNode listGetNext(ListNode node);
Pointer listNodeValue(ListNode node);
int listSize(List list);
void listSetDestroyValue(List list, DestroyFunc destroyValue);

