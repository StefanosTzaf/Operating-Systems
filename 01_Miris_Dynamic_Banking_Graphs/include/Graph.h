#include "Map.h"
typedef struct graph* Graph;
typedef struct graph_node* GraphNode;
typedef struct edge* Edge;


//------------------------------------------------------- Functions for lists ------------------------------------------

//O(1)
//compare function for comparing node ids in the node list
int compareGraphNodes(Pointer a, Pointer b);
//O(1)
//compare function for edges
int compareEdges(Pointer a, Pointer b);
//O(1)
//compare function for comparing node ids in hash table
int compareMapNodes(Pointer a, Pointer b);

//O(n + m), where n is GraphNode list nodes and m is edges
//destroys value of nodes in graph list, i.e. GraphNode
void destroyGraphListNode(Pointer nodeToDelete);

//O(n), where n is edges in an edge list
//destroys value of nodes in edge lists
void destroyEdge(Pointer edgeToDelete);

//O(1)
//destroys value of nodes in hash table lists
void destroyMapNodes(Pointer value);

//---------------------------------------------------------------------------------------------------------------------------------


//O(1)
Graph graphCreate();

//O(1)
void graphAddNode(Graph graph, char* id, Map map);
void addEdge(Graph graph, char* dateOfTransmission, int amount, char* id1, char* id2, Map map);

//O(n + m) due to destroyGraphListNode
void removeGraphNode(char* id, Map map, Graph graph);

//O(n) due to findEdge
void removeEdge(char* id1, char* id2, Map map);

//finds edge based only on node ids (needed by removeEdge)
//O(n), where n is number of outgoing edges of the node
Edge findEdge(char* id1,char* id2, Map map);

//O(n)
bool modifyEdge(char* id1, char* id2, char* date, int amount,char* date2, int amount2, Map map);

//O(n), where n is edges
void displayOutgoingEdges(char* id, Map map);
void displayIncomingEdges(char* id, Map map);

//O(n*m)
void destroyGraph(Graph graph);
void printToFile(Graph graph, FILE* file);

//O(m + n), known DFS complexity
void findCircles(char* id,Graph graph, Map map, int minSum, bool flag);
void dfsPrintingCircles(GraphNode node, GraphNode startNode, List list, int minSum);
void findPath(Graph graph, char* id1, char* id2, Map map);
void dfsPath(GraphNode node, char* destination, List list, Map map, bool* found);