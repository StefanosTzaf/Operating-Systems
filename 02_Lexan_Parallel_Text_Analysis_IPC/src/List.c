#include "List.h"

// doubly linked list
// The list will be a pointer to this struct
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
	new->next = NULL;	// add the node at the end of the list
	new->prev = list->last;	

	// If there is a node in the list
	if (list->last != NULL){
		list->last->next = new;
	}
	// if the list is empty
	else{
		list->head = new;
	}
	list->last = new;
	list->size++;
}


void listDeleteNode(List list, Pointer value){
	ListNode node = listFind(list, value);
	
	// Check if the list is empty
	if (node == NULL) {
		return;
	}

	ListNode previousNode = node->prev;
	ListNode nextNode = node->next;

	// The only node in the list is the one being deleted
	if (previousNode == NULL && nextNode == NULL) {
		list->head = NULL;
		list->last = NULL;
	}

	// the node was at the beginning but has a next node
	else if (previousNode == NULL) {
		list->head = nextNode;
		nextNode->prev = NULL;
	}

	// the node was at the end but has a previous node
	else if (nextNode == NULL) {
		list->last = previousNode;
		previousNode->next = NULL;
	}

	else {
		previousNode->next = nextNode;
		nextNode->prev = previousNode;
	}

	// Free the memory of the node being deleted
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
	// If the list has only one node
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
		// destroy each node based on the destroyValue function
		// (the Pointer may point to different data types so each time we must handle free differently to avoid leaks)
		list->destroyValue(node->value);
		node = next;
	}

	// Free the memory allocated for the nodes
	node = list->head;
	while (node != NULL) {
		ListNode next = node->next;
		free(node);
		node = next;
	}

	// Free the memory allocated for the struct
	free(list);
}


// Search the list based on the compare function
ListNode listFind(List list, Pointer value) {
	// traverse the entire list, call compare until it returns 0
	for (ListNode node = list->head; node != NULL; node = node->next){
		if (list->compare(value, listNodeValue(node)) == 0)
			return node;
	}
	return NULL;
}

// to prevent access outside the file to elements of the struct list, we create getters that will be needed
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
