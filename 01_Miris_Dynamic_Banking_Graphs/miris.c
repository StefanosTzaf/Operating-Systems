#include "Graph.h"
#include <getopt.h>
#include <stdbool.h>

int main(int argc, char *argv[]){


  int option;

  //To store input and output file names from command line
  char* inputFile = NULL;
  char* outputFile = NULL;

  if(argc != 5){
    fprintf(stderr, "Error : run with format -i inputfile -o outputfile\n");
    return 1;
  }

  while((option = getopt(argc, argv, "i:o:")) != -1){
    if(option == 'i'){
      inputFile = optarg;
    }
    else if(option == 'o'){
      outputFile = optarg;
    } 
    else{
      return 1;
    }
  }

  FILE* file;
  file = fopen(inputFile, "r");
  if(file == NULL){
    fprintf(stderr, "Error opening file\n");
    return 1;
  }

  char line[256];     //for reading one line from file
  int counter = 0;    //how many lines file has, used for hash table size
  while (fgets(line, sizeof(line), file) != NULL) {
      counter++;
  }

  Graph graph = graphCreate();
  Map map = mapCreate(compareMapNodes, destroyMapNodes, counter);

  rewind(file);   //from start of file
  //Using strtok (as discussed in tutorial) we read file data
  //separated by spaces. First call passes the line,
  //then NULL (function continues from previous position).
  while (fgets(line, sizeof(line), file) != NULL) {
    char* id1;
    char* id2;
    int amount;
    char* date;
    id1 = strtok(line, " ");
    id2 = strtok(NULL, " ");
    //convert to integer
    amount = atoi(strtok(NULL, " "));
    date = strtok(NULL, "\n");
    addEdge(graph, date, amount, id1, id2, map);
  }
  fclose(file);


//-----------------------------------------------------prompt---------------------------------------------------------

  char* token;
  bool exit = false; 
  do{
    printf("Miris waiting for a command :\n");
    //Read with getchar because we do not know user input length
    //for example, user may type many nodes for insert
    char ch;
    char* command = NULL;
    int size = 0;
    int capacity = 1;
    command = malloc(sizeof(char)* capacity);
    
    while(((ch = getchar()) != '\n') && (ch!= EOF)){
      if(size >= capacity - 1){
        //double buffer size to avoid too many realloc calls
        capacity *= 2;
        command = realloc(command, capacity * sizeof(char));
      }
      command[size++] = ch;
    }
    command[size] = '\0';
    
    //if Enter is pressed with empty input
    if(strcmp(command, "") == 0){
      free(command);
      continue;
    }

    //copy string because a second strtok pass is needed, see insert
    char* commandCopy;
    commandCopy = malloc(strlen(command) + 1);
    strcpy(commandCopy, command);

    token = strtok(command, " ");


    //--------------------------------------------------------------- 1 -------------------------------------------------------
    if(strcmp(token, "i") == 0 || strcmp(token, "insert") == 0){

      token = strtok(NULL, " ");
      //if first argument after insert is NULL, format is invalid
      if(token == NULL){
        printf("   Format error:\n");
        printf("   Command Name : i Ni [Nj Nk ...] or insert Ni [Nj Nk ...]\n\n");
        free(command);
        free(commandCopy);
        continue;
      }

      //check whether any node with this id already exists
      bool exists = false;
      while(token != NULL){
        if(mapFind(map, token) != NULL){
          printf("   IssueWith: %s (already exists)\n", token);
          exists = true;
          break;
        }
        token = strtok(NULL, " ");
      }

      if(exists){
        free(command);
        free(commandCopy);
        continue;
      }

      //skip command token i (we use command copy to ensure content
      //matches original command and traverse string from start safely)

      printf("   Insert into the graph structure 1 or more nodes\n   with specific STRING ids.\n\n");
      token = strtok(commandCopy, " ");
      token = strtok(NULL, " ");

      printf("   Succ: ");
      while(token != NULL){
        printf(" %s", token);
        graphAddNode(graph, token, map);
        token = strtok(NULL, " ");

      }
      printf("\n\n");
      
    }

    //--------------------------------------------------------------- 2 -------------------------------------------------------
    else if(strcmp(token, "n") == 0 || strcmp(token, "insert2") == 0){

      token = strtok(NULL, " ");
      char* token2 = strtok(NULL, " ");
      char* sum = strtok(NULL, " ");
      char* date = strtok(NULL, " ");
      //there should be nothing after date
      char* next = strtok(NULL, " ");

      if(token == NULL || token2 == NULL || sum == NULL || date == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name : n Ni Nj amount date or insert2 n Ni Nj sum date\n\n");
      }
      else if(strcmp(token, token2) == 0){
        printf("   Transaction with the same origin and destination not allowed\n\n");
      }
      else{
        printf("   introduce an edge with direction from %s to %s with label\n   %s + %s if either %s or %s does not exist in the graph,\n   do the appropriate node insertion first.\n\n", token, token2, sum, date, token, token2);
        addEdge(graph, date, atoi(sum), token, token2, map);
      }
    }


    //--------------------------------------------------------------- 3 --------------------------------------------------------
    else if(strcmp(token,"d") == 0 || strcmp(token, "delete") == 0){
      token = strtok(NULL, " ");
      //if first argument after delete is NULL, format is invalid
      if(token == NULL){
        printf("   Format error:\n");
        printf("   Command Name : d Ni [Nj Nk ...] or delete Ni [Nj Nk ...]\n\n");
        free(command);
        free(commandCopy);
        continue;
      }
      printf("   delete from graph listed nodes Ni, Nj, Nk, etc\n\n");

      //check whether every node id exists
      bool exists = false;
      while(token != NULL){
        if(mapFind(map, token) == NULL){
          printf("   IssueWith: %s (no exists)\n", token);
          exists = true;
          break;
        }
        token = strtok(NULL, " ");
      }

      if(exists){
        free(command);
        free(commandCopy);
        continue;
      }

      //same idea as insert
      token = strtok(commandCopy, " ");
      token = strtok(NULL, " ");

      while(token != NULL){
        removeGraphNode(token, map, graph);
        token = strtok(NULL, " ");

      }
      printf("\n\n");

    }

    //--------------------------------------------------------------- 4 --------------------------------------------------------
    else if(strcmp(token, "l" ) == 0 || strcmp(token, "delete2") == 0){

      token = strtok(NULL, " ");
      char* token2 = strtok(NULL, " ");
      //there should be nothing after second argument
      char* next = strtok(NULL, " ");

      if(next != NULL || token == NULL || token2 == NULL){
        printf("   Format error:\n");
        printf("   Command Name : l Ni Nj or delete2 Ni Nj\n\n");
      }

      else if(findEdge(token, token2, map) == NULL){
        printf("   Edge between %s - %s not found\n\n", token, token2);
      }

      else{
        printf("   remove the edge between %s and %s; if there are\n   more than one edges, remove one of the edges.\n\n", token, token2);
        removeEdge(token, token2, map);
      }
    }


    //--------------------------------------------------------------- 5 --------------------------------------------------------
    else if(strcmp(token, "m") == 0 || strcmp(token, "modify") == 0){
      char* id1  = strtok(NULL, " ");
      char* id2 = strtok(NULL, " ");
      char* sum = strtok(NULL, " ");
      char* sum2 = strtok(NULL, " ");
      char* date = strtok(NULL, " ");
      char* date2 = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(id1 == NULL || id2 == NULL || sum == NULL || sum2 == NULL || date2 == NULL || date == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name :m Ni Nj sum sum1 date date1 or modify Ni Nj sum sum1 date date1\n\n");
      }
      else if(modifyEdge(id1, id2, date, atoi(sum), date2, atoi(sum2), map) == 1){
        printf("   Non-existing edge:\n\n" );
      }
      else{
        printf("   update the values of a specific edge between %s and %s\n\n", id1, id2);
      }
    }

    //--------------------------------------------------------------- 6 --------------------------------------------------------

    else if(strcmp(token, "f") == 0 || strcmp(token, "find") == 0){
      token = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name : f Ni or find Ni\n\n");
      }

      else if(mapFind(map, token) == NULL){
        printf("   Non-existing node %s \n\n", token);
      }

      else{
        printf("   find all outgoing edges from %s\n\n", token);
        displayOutgoingEdges(token, map);
      }

    }

    //--------------------------------------------------------------- 7 --------------------------------------------------------

    else if(strcmp(token, "r") == 0 || strcmp(token, "receiving")==0){
      token = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name : r Ni or receiving Ni\n\n");
      }

      else if(mapFind(map, token) == NULL){
        printf("   Non-existing node %s \n\n", token);
      }

      else{
        printf("   find all ingoing edges from %s\n\n", token);
        displayIncomingEdges(token, map);
      }      
    }

    //--------------------------------------------------------------- 8 --------------------------------------------------------
    else if(strcmp(token, "c") == 0 || strcmp(token, "circlefind") == 0){
      token = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name : c Ni or circlefind Ni\n\n");
      }

      else if(mapFind(map, token) == NULL){
        printf("   Non-existing node %s \n\n", token);
      }

      else{
        printf("   find all circles that contain node %s\n\n", token);
        //Simple cycles without minimum amount
        findCircles(token, graph, map, 0, 0);
        
      }
    }

    //--------------------------------------------------------------- 9 --------------------------------------------------------
    
    else if(strcmp(token, "fi") == 0 || strcmp(token, "findcircles") == 0){
      token = strtok(NULL, " ");
      char* sum = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || sum == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name : c Ni k or circlefind Ni k\n\n");
      }

      else if(mapFind(map, token) == NULL){
        printf("   Non-existing node %s \n\n", token);
      }

      else{
        printf("   find circular relationships in which %s is involved and moves at least k units of funds.\n", token);
        findCircles(token, graph, map, atoi(sum), 1);
      }
    }


    //--------------------------------------------------------------- 11 --------------------------------------------------------
    else if(strcmp(token, "o") == 0 || strcmp(token, "connected") == 0){
      token = strtok(NULL, " ");
      char* id2 = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || id2 == NULL || next != NULL){
        printf("   Format error:\n");
        printf("   Command Name : o Ni Nj or connected Ni Nj\n\n");
      }

      else if(mapFind(map, token) == NULL){
        printf("   Non-existing node %s \n\n", token);
      }

      else if(mapFind(map, id2) == NULL){
        printf("   Non-existing node %s \n\n", id2);
      }

      else{
        findPath(graph, token, id2, map);
      }
    }


    //--------------------------------------------------------------- 12 --------------------------------------------------------
    else if(strcmp(token, "e") == 0 || strcmp(token, "exit") == 0){
      printf("terminate the program.\n");
      exit = true;
    }
    else{
      printf("Unrecognized command: %s\n",token);
    }

    free(command);
    free(commandCopy);
  }
  while(!exit);


  file = fopen(outputFile, "w");
  //TODO MAKE THE OUTPUT FILE IF NOT EXISTS AND IF NOT GIVEN
  if(file == NULL){
    fprintf(stderr, "Error opening file\n");
    return 1;
  }
  printToFile(graph, file);
  fclose(file); 
  destroyGraph(graph);
  mapDestroy(map);

  return 0;
}