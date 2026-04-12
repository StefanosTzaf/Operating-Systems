//impemantation of set with binary search tree

#include <stdlib.h>
#include "Set.h"

struct set {
	SetNode root;
	int size;
	CompareFunc compare;
	DestroyFunc destroyValue;
};


struct set_node {
	SetNode left, right;
	Pointer value;
};


Set setCreate(CompareFunc compare, DestroyFunc destroyValue) {
	Set set = malloc(sizeof(*set));
	set->root = NULL;
	set->size = 0;
	set->compare = compare;
	set->destroyValue = destroyValue;
	return set;
}


int setSize(Set set) {
	return set->size;
}


//recursive function to insert a node with a value in the set
SetNode setInsertNode(Set set, Pointer value, SetNode subtreeRoot) {

	//if the subtree is empty
	if(subtreeRoot == NULL) {
		SetNode node = malloc(sizeof(*node));
		node->left = NULL;
		node->right = NULL;
		node->value = value;
		set->size++;
		return node;
	}
	//if it is not, we have to descend to the right or left subtree
	else{
		int compareValue = set->compare(value, subtreeRoot->value);
		//if the value is already in the set(based to the compare function) DO NOT ADD it again 
		if(compareValue == 0) {
			//do nothing
		}
		else if(compareValue < 0) {
			subtreeRoot->left = setInsertNode(set, value , subtreeRoot->left);

		}
		else {
			subtreeRoot->right = setInsertNode(set, value, subtreeRoot->right);
		}

	}
	return subtreeRoot;

}


void setInsert(Set set, Pointer value){
	set->root = setInsertNode(set, value, set->root);
}


void nodeDestroy(SetNode node, DestroyFunc destroyValue) {
	if (node == NULL){
		return;
	}
	
	nodeDestroy(node->left, destroyValue);
	nodeDestroy(node->right, destroyValue);

	if (destroyValue != NULL){
		destroyValue(node->value);
	}

	free(node);
}


void setDestroy(Set set) {
	nodeDestroy(set->root, set->destroyValue);
	free(set);
}


//return the minimum of the subtree with root node based to the compare function
SetNode setMin(SetNode node) {
	//if the left subtree exists , continue there
	if(node != NULL && node->left != NULL){
		return setMin(node->left);
	}
	else{
		return node;
	}
}

SetNode setMax(SetNode node) {
	if(node != NULL && node->right != NULL){
		return setMax(node->right);
	}
	else{
		return node;
	}
}

SetNode setFirst(Set set) {
	return setMin(set->root);
}

SetNode setLast(Set set) {
	return setMax(set->root);
}

Pointer setNodeValue(Set set, SetNode node) {
	return node->value;
}


// returns the previous node (based to compare) of the currentNode. It returns NULL if it is the 
// MIN of the set
SetNode nodeFindPrevious(SetNode root, Set set, SetNode currentNode) {
	if (root == currentNode) {

		//if target node(the node that we are looking for its previous) its previous would be in
		//the left subtree.If there is no left subtree then the target is the min of the set
		return nodeFindMax(root->left);

	} 
	else if (set->compare(currentNode->value, root->value) < 0) {
		//target is in the left subtree so we search there
		return nodeFindPrevious(root->left, set, currentNode);

	}
	//if the target is in the right subtree then its previous would be in the right subtree
	//OR the root is the previous of the target
	else {
		SetNode res = nodeFindPrevious(root->right, set, currentNode);
		if(res == NULL) {
			return root;
		}
		else {
			return res;
		}
	}
}


//Returns the maximum of the subtree with root node (needed for the previous function)
SetNode nodeFindMax(SetNode node) {
	if(node != NULL && node->right != NULL){
		return nodeFindMax(node->right);
	}
	else{
		return node;
	}
}

SetNode getRootNode(Set set){
	return set->root;
}