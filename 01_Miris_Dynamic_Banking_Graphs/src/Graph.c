#include "Graph.h"

//------------------------------------------------------Structure definitions-------------------------------------------------------
//The graph consists of a list of GraphNodes
struct graph{
    List nodes;
    int size;
};

struct graph_node{
    char* id;
    //List of edges that start from this node and end at another node
    List outgoingEdges;
    //List of edges that end at this node
    List incomingEdges;
    bool visited;
};

//An edge consists of the transaction date, transaction amount,
//and pointers to destination and origin nodes
struct edge{
    char* dateOfTransaction;
    int amount;
    GraphNode nodeDestination;
    GraphNode nodeOrigin;
};
//------------------------------------------------------------------------------------------------------------------------------




//------------------------------------------------------Functions passed to lists---------------------------------------------------
//Compare function for node id comparison
int compareGraphNodes(Pointer a, Pointer b){
    char* id1 = ((GraphNode)a)->id;
    char* id2 = ((GraphNode)b)->id;
    return strcmp(id1, id2);
}

//Compare edges based on origin and destination nodes
int compareEdges(Pointer a, Pointer b){
    Edge edge1 = a;
    Edge edge2 = b;
    if((strcmp(edge1->nodeDestination->id, edge2->nodeDestination->id) == 0) && (strcmp(edge1->nodeOrigin->id, edge2->nodeOrigin->id)==0)){
        return 0;
    }
    return 1;
}

//Compare a Map node (whose value is a GraphNode) against a key (id)
int compareMapNodes(Pointer a, Pointer b){
    GraphNode node =  mapNodeValue((MapNode)b);
    return strcmp((char*)a, node->id);
}

//Just free the pointer; GraphNode internals are freed by destroyGraphListNode
void destroyMapNodes(Pointer value){
    free(value);
}


//Function that tells the GraphNode list how to destroy each node
void destroyGraphListNode(Pointer nodeToDelete){
    GraphNode node = nodeToDelete;

    // fixed , we don't dostroy the 2 lists because when an edge is destroyd from an edge list,
    // it is also removed from the other list, so we would have double free if we destroy both lists here.
    while (listSize(node->outgoingEdges) > 0) {
        Edge e = listNodeValue(listGetFirst(node->outgoingEdges));
        destroyEdge(e); 
    }
    while (listSize(node->incomingEdges) > 0) {
        Edge e = listNodeValue(listGetFirst(node->incomingEdges));
        destroyEdge(e);
    }
    
    // lists are now empty, we can destroy them
    listSetDestroyValue(node->incomingEdges, NULL);
    listSetDestroyValue(node->outgoingEdges, NULL);
    
    listDestroy(node->incomingEdges);
    listDestroy(node->outgoingEdges);
    
    free(node->id);
    free(node);
}

//When we want to remove an edge:
void destroyEdge(Pointer edgeToDelete){
    Edge edge = edgeToDelete;
    GraphNode nodeDestination = edge->nodeDestination;
    GraphNode nodeOrigin = edge->nodeOrigin;


    if(nodeDestination->incomingEdges != NULL){
        //Set destroy value to null so the list does not try to
        //delete the edge payload as well, since we do it in this function.
        //We only want it to remove the ListNode and reconnect nodes properly.
        listSetDestroyValue(nodeDestination->incomingEdges, NULL);
        listDeleteNode(nodeDestination->incomingEdges, edgeToDelete);
        listSetDestroyValue(nodeDestination->incomingEdges, destroyEdge);
    }

    if(nodeOrigin->outgoingEdges != NULL){
        listSetDestroyValue(nodeOrigin->outgoingEdges, NULL);
        listDeleteNode(nodeOrigin->outgoingEdges, edgeToDelete);
        listSetDestroyValue(nodeOrigin->outgoingEdges, destroyEdge);
    }
    free(edge->dateOfTransaction);
    free(edge);

}


//------------------------------------------------------------------------------------------------------------------------------



//------------------------------------------------------Graph functions---------------------------------------------------

Graph graphCreate(){
    Graph graph = malloc(sizeof(*graph));
    //Create a list of nodes
    graph->nodes = listCreate(destroyGraphListNode, compareGraphNodes);
    graph->size = 0;
    return graph;
}


void graphAddNode(Graph graph, char* id, Map map){
    //Create a node
    GraphNode node = malloc(sizeof(*node));
    node->id = malloc(strlen(id) + 1);
    strcpy(node->id, id);
    node->visited = false;
    //Create edge lists. We use destroyEdge in both lists;
    //edge deletion is controlled to avoid double free by temporarily
    //disabling destroy where needed.
    node->incomingEdges = listCreate(destroyEdge, compareEdges);
    node->outgoingEdges = listCreate(destroyEdge, compareEdges);

    //Insert node in the graph node list
    listInsert(graph->nodes, node);
    //Also update hash table
    mapInsert(map, node->id, node);//Store GraphNode in map
    graph->size++;

}

void addEdge(Graph graph, char* dateOfTransaction, int amount, char* id1, char* id2, Map map){
    //Find node with id1, returns GraphNode*
    GraphNode node1 = mapFind(map, id1);
    GraphNode node2 = mapFind(map, id2);

    //If a node does not exist, create it
    if (node1 == NULL) {
        graphAddNode(graph, id1, map);
        node1 = mapFind(map, id1);

    }
    if (node2 == NULL) {
        graphAddNode(graph, id2, map);
        node2 = mapFind(map, id2);
    }
    //Create an edge with dateOfTransaction
    Edge edge = malloc(sizeof(*edge));
    edge->dateOfTransaction = malloc(strlen(dateOfTransaction) + 1);
    strcpy(edge->dateOfTransaction, dateOfTransaction);
    edge->amount = amount;
    edge->nodeDestination = node2;
    edge->nodeOrigin = node1;
    
    
    //Insert edge into outgoing list of node1
    listInsert(node1->outgoingEdges, edge);
    //Insert edge into incoming list of node2
    listInsert(node2->incomingEdges, edge);
}


void removeGraphNode(char* id, Map map, Graph graph){
    //Tell hash table to stop pointing to this node because it no longer exists
    GraphNode node = mapFind(map, id);
    if(node == NULL){
        return;
    }
    mapRemove(map, node->id);

    listDeleteNode(graph->nodes, node);
    
    graph->size--;
}


void removeEdge(char* id1, char* id2, Map map){

    Edge edgeToRemove = findEdge(id1, id2, map);

    if(edgeToRemove != NULL){

        destroyEdge(edgeToRemove);
    }

}

Edge findEdge(char* id1,char* id2, Map map){
    GraphNode node1 = mapFind(map, id1);
    GraphNode node2 = mapFind(map, id2);
    if(node1 == NULL || node2 == NULL){
        return NULL;
    }
    Edge temp = malloc(sizeof(*temp));
    temp->nodeDestination = node2;
    temp->nodeOrigin = node1;

    // fixed: we search one time for the edge in node1 outgoing list
    ListNode foundNode = listFind(node1->outgoingEdges, temp);
    free(temp);
    if(foundNode == NULL){
        return NULL;
    } else {
        return listNodeValue(foundNode);
    }
}

bool modifyEdge(char* id1, char* id2, char* date, int amount,char* date2, int amount2, Map map){
    //findEdge cannot help here because it compares only by ids
    GraphNode origin = mapFind(map, id1);
    GraphNode destination = mapFind(map, id2);
    if(origin == NULL || destination == NULL){
        return 1;
    }
    //Traverse only origin outgoing list (we search only for id1->id2 edge)
    for(ListNode node = listGetFirst(origin->outgoingEdges); node != NULL; node = listGetNext(node)){
        
        Edge edge = listNodeValue(node);
        if(strcmp(edge->nodeDestination->id, destination->id) == 0
            && strcmp(edge->dateOfTransaction, date) == 0 
            && edge->amount == amount){
            
            free(edge->dateOfTransaction);
            edge->dateOfTransaction = malloc(strlen(date2) + 1);
            strcpy(edge->dateOfTransaction, date2);
            edge->amount = amount2;
            return 0;
        }
    }
    return 1;
}

void displayOutgoingEdges(char* id, Map map){
    GraphNode node = mapFind(map, id);
    if(node == NULL){
        return;
    }
    ListNode outgoingEdges = listGetFirst(node->outgoingEdges);
    while(outgoingEdges != NULL){
        Edge edge = listNodeValue(outgoingEdges);
        printf("   %s %s %d %s\n",edge->nodeOrigin->id, edge->nodeDestination->id, edge->amount, edge->dateOfTransaction);
        outgoingEdges = listGetNext(outgoingEdges);
    }
}


void displayIncomingEdges(char* id, Map map){
    GraphNode node = mapFind(map, id);
    if(node == NULL){
        return;
    }
    ListNode incomingEdges = listGetFirst(node->incomingEdges);
    while(incomingEdges != NULL){
        Edge edge = listNodeValue(incomingEdges);
        printf("   %s %s %d %s\n",edge->nodeOrigin->id, edge->nodeDestination->id, edge->amount, edge->dateOfTransaction);
        incomingEdges = listGetNext(incomingEdges);
    }
}


void printToFile(Graph graph, FILE* file){
    ListNode listNode = listGetFirst(graph->nodes);
    //For each graph node
    while(listNode != NULL){

        GraphNode node = listNodeValue(listNode);
        //Print only outgoing edges so each edge is not printed twice
        if(listSize( node->outgoingEdges) == 0 && listSize(node->incomingEdges) == 0){
            fprintf(file, "%s (No transactions)\n", node->id);
        
        }
        //Each outgoing edge (no need to check incoming; would print twice)
        ListNode outgoingEdges = listGetFirst(node->outgoingEdges);

        while(outgoingEdges != NULL){
            Edge edge = listNodeValue(outgoingEdges);
            fprintf(file,"%s ", edge->nodeOrigin->id);
            fprintf(file,"%s ", edge->nodeDestination->id);
            fprintf(file, "%d ", edge->amount);
            fprintf(file, "%s\n", edge->dateOfTransaction);

            outgoingEdges = listGetNext(outgoingEdges);
        }
        listNode = listGetNext(listNode);
    }
}

void destroyGraph(Graph graph){
    //All cleanup is handled by destroyGraphListNode passed to list
    listDestroy(graph->nodes);
    free(graph);
}


//If flag is 0 we search for simple cycles, otherwise cycles with minimum total amount
void findCircles(char* id, Graph graph, Map map, int minSum, bool flag) {
    GraphNode startNode = mapFind(map, id);
    if (startNode == NULL) {
        printf("   Non-existing node: %s\n\n", id);
        return;
    }

    List list = listCreate(NULL, compareGraphNodes);
    //8
    if(!flag){
        dfsPrintingCircles(startNode, startNode, list, -1);
        printf("\n");
    }
    //9
    else{
        dfsPrintingCircles(startNode, startNode, list, minSum);
        printf("\n");
    }
    listDestroy(list);
}


void dfsPrintingCircles(GraphNode node, GraphNode startNode, List list, int minSum) {

    node->visited = true;
    //If there is no edge, then no cycle exists
    if(node->outgoingEdges == NULL){
        return;
    }
    listInsert(list, node);
    bool foundCircle = false;   
    //For each outgoing edge
    for (ListNode outgoingEdges = listGetFirst(node->outgoingEdges);
        outgoingEdges != NULL; outgoingEdges = listGetNext(outgoingEdges)){

        Edge edge = listNodeValue(outgoingEdges);
        //For cycles with minimum amount: if a path edge has amount smaller than min,
        //skip it. For unrestricted cycles (8), minSum is -1 so this never applies.
        if(edge->amount < minSum){
            continue;
        }
        GraphNode child = edge->nodeDestination;

        //If reached node is the start node, a cycle was found
        if (strcmp(child->id, startNode->id) == 0) {
            printf("   ");
            foundCircle = true;
            listInsert(list, child);
            ListNode circleNode = listGetFirst(list);
    	    //Print cycle
            while (circleNode != NULL) {
                GraphNode node1 = listNodeValue(circleNode);
                printf("%s ", node1->id);
                circleNode = listGetNext(circleNode);
                if(circleNode != NULL){
                    printf("-> ");
                }
            }

            printf("\n");
            //Remove node; we want only simple cycles, otherwise it would find cycles like
            //2->3->4->2->6->2
            listRemoveLast(list);
        } 
        
        else if (!child->visited) {
            //If child has not been visited, recurse into it
            dfsPrintingCircles(child, startNode, list, minSum);
        }

        //If current node is the start node and a cycle was found,
        //then stop (we want simple cycles)
        if((strcmp(node->id, startNode->id) == 0 ) && foundCircle){
            break;
        }
    }

    //Reset for future searches
    node->visited = false;
    listDeleteNode(list, node);
}

void findPath(Graph graph, char* id1, char* id2, Map map){
    GraphNode node1 = mapFind(map, id1);

    List list = listCreate(NULL, compareGraphNodes);
    bool found = false;
    dfsPath(node1, id2, list, map, &found);
    if(!found){
        printf("   No path found\n");
    }
    listDestroy(list);
    
}

void dfsPath(GraphNode node, char* destination, List list, Map map, bool* found){
    node->visited = true;
    //If there is no edge, then no path exists
    if(node->outgoingEdges == NULL){
        return;
    }
    listInsert(list, node);  
    //For each outgoing edge
    for (ListNode outgoingEdges = listGetFirst(node->outgoingEdges);
        outgoingEdges != NULL; outgoingEdges = listGetNext(outgoingEdges)){

        Edge edge = listNodeValue(outgoingEdges);
        GraphNode child = edge->nodeDestination;

        //If reached node is destination, we found a path
        if (strcmp(child->id, destination) == 0) {
            printf("   ");
            (*found) = true;
            listInsert(list, child);
            ListNode pathNode = listGetFirst(list);
    	    //Print path
            while (pathNode != NULL) {
                GraphNode node1 = listNodeValue(pathNode);
                printf("%s ", node1->id);
                pathNode = listGetNext(pathNode);
                if(pathNode != NULL){
                    printf("-> ");
                }
            }

            printf("\n");
            listRemoveLast(list);
        } 
        
        else if (!child->visited) {
            //If child has not been visited, recurse into it
            dfsPath(child, destination, list, map, found);
        }

        if(*found){
            break;
        }
    }
    node->visited = false;
    listDeleteNode(list, node);
}
