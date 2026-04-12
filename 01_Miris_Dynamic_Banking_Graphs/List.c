#include "List.h"

//Doubly linked list
//The list will be a pointer to this struct
struct list {
	ListNode head;
	ListNode last;
	int size;
	DestroyFunc destroyValue;
	CompareFunc compare;
};

struct list_node {
	ListNode next;
	ListNode prev;
	Pointer value;
};


List listCreate(DestroyFunc destroyValue, CompareFunc compare) {
	List list = malloc(sizeof(*list));
	list->size = 0;
	list->last = NULL;
	list->head = NULL;
	list->destroyValue = destroyValue;
	list->compare = compare;
	return list;
}

void listInsert(List list, Pointer value) {
	ListNode new = malloc(sizeof(*new));
	new->value = value;
	new->next = NULL;	//Insert node at the end of the list
	new->prev = list->last;	

	//If there is at least one node in the list
	if (list->last != NULL){
		list->last->next = new;
	}
	///If the list is empty
	else{
		list->head = new;
	}
	list->last = new;
	list->size++;
}


void listDeleteNode(List list, Pointer value){
	ListNode node = listFind(list, value);
	
	// Check if list is empty
    if (node == NULL) {
	    return;
    }

	ListNode previousNode = node->prev;
	ListNode nextNode = node->next;

	//The only node in the list is the one being deleted
	if (previousNode == NULL && nextNode == NULL) {
		list->head = NULL;
		list->last = NULL;
	}

	//Node was at the beginning and has a next node
	else if (previousNode == NULL) {
		list->head = nextNode;
		nextNode->prev = NULL;
	}

	//Node was at the end and has a previous node
	else if (nextNode == NULL) {
		list->last = previousNode;
		previousNode->next = NULL;
	}

	else {
		previousNode->next = nextNode;
		nextNode->prev = previousNode;
	}

	// Free memory of deleted node
	if(list->destroyValue != NULL){
		list->destroyValue(node->value);
	}

	if(node != NULL){
		free(node);
		node = NULL;
	}

	list->size--;
}


void listRemoveLast(List list){
	ListNode last = list->last;
	if(last == NULL){
		return;
	}
	ListNode previous = last->prev;
	//If list has only one node
	if(previous == NULL){
		list->head = NULL;
	}
	else{
		previous->next = NULL;
	}
	list->last = previous;
	list->size--;
	if(list->destroyValue != NULL){
		list->destroyValue(last->value);
	}
	free(last);
}


void listDestroy(List list) {
	ListNode node = list->head;
	while (node != NULL) {
		ListNode next = node->next;
		//Destroy each node value based on destroyValue function
		//(Pointer may reference different data types, so freeing differs each time to avoid leaks)
		list->destroyValue(node->value);
		node = next;
	}

	//Free memory reserved for nodes
	node = list->head;
	while (node != NULL) {
		ListNode next = node->next;
		free(node);
		node = next;
	}

	//Free memory reserved for the struct
	free(list);
}


//Search in list based on compare
ListNode listFind(List list, Pointer value) {
	//Traverse entire list, call compare until it returns 0
	for (ListNode node = list->head; node != NULL; node = node->next){
		if (list->compare(value, listNodeValue(node)) == 0)
			return node;
	}
	return NULL;
}

//To prevent external access to struct list fields, we provide needed getters
ListNode listGetFirst(List list){
	return list->head;
}

ListNode listGetLast(List list){
	return list->last;
}

ListNode listGetNext(ListNode node){
	return node->next;
}

Pointer listNodeValue(ListNode node){
	return node->value;	
}

int listSize(List list){
	return list->size;
}

void listSetDestroyValue(List list, DestroyFunc destroyValue){
	list->destroyValue = destroyValue;
}