//--------------------------------------Doubly linked list implementation-----------------------------------------
#include <stdlib.h>
#include <stdio.h>

typedef void* Pointer;
//We want our list to be generic, so it stores pointers to void elements (either GraphNode or Edge)
typedef struct list* List;
typedef struct list_node* ListNode;

//We use a function pointer to compare 2 elements, it returns:
//0 when they are equal based on the function (not necessarily *a == *b,
//we may compare structs, so compare can differ, and that is its value)
//Used in listFind
typedef int (*CompareFunc)(Pointer a, Pointer b);

//Similarly, Destroy differs depending on what the pointer references.
//For example, in this assignment we use three list types that allocate different memory.
//destroy is a field of struct list
typedef void (*DestroyFunc)(Pointer value);

//(O(1))
List listCreate(DestroyFunc destroyValue, CompareFunc compare);

//(O(1))
void listInsert(List list, Pointer value);

//(O(n)) due to ListFind
void listDeleteNode(List list, Pointer value);
//O(1)
void listRemoveLast(List list);

//(O(n))
void listDestroy(List list);

//(O(n))
ListNode listFind(List list, Pointer value);

//(O(1))
ListNode listGetLast(List list);
ListNode listGetFirst(List list);
ListNode listGetNext(ListNode node);
Pointer listNodeValue(ListNode node);
int listSize(List list);
void listSetDestroyValue(List list, DestroyFunc destroyValue);
