#include "Graph.h"
#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Συνάρτηση για την εκτύπωση του Μενού Επιλογών
void printMenu() {
    printf("\n============================= MIRIS COMMAND MENU =============================\n");
    printf(" i  / insert Ni [Nj Nk ...]           : Insert one or more nodes\n");
    printf(" n  / insert2 Ni Nj amount date       : Insert a transaction (edge)\n");
    printf(" d  / delete Ni [Nj Nk ...]           : Delete one or more nodes\n");
    printf(" l  / delete2 Ni Nj                   : Delete an edge between Ni and Nj\n");
    printf(" m  / modify Ni Nj sum sum1 date date1: Modify an existing edge\n");
    printf(" f  / find Ni                         : Find all outgoing edges from Ni\n");
    printf(" r  / receiving Ni                    : Find all incoming edges to Ni\n");
    printf(" c  / circlefind Ni                   : Find all simple cycles containing Ni\n");
    printf(" fi / findcircles Ni k                : Find cycles containing Ni with min amount k\n");
    printf(" o  / connected Ni Nj                 : Find if a path exists from Ni to Nj\n");
    printf(" h  / help                            : Print this menu again\n");
    printf(" e  / exit                            : Save to file and exit the program\n");
    printf("==============================================================================\n\n");
}

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
  printMenu();

  char* token;
  bool exit = false; 
  do{
    printf("Miris waiting for a command :\n> ");
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

    if (token == NULL) {
      free(command);
      free(commandCopy);
      continue;
    }

    //--------------------------------------------------------------- 1 -------------------------------------------------------
    if(strcmp(token, "i") == 0 || strcmp(token, "insert") == 0){

      token = strtok(NULL, " ");
      //if first argument after insert is NULL, format is invalid
      if(token == NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : i Ni [Nj Nk ...] or insert Ni [Nj Nk ...]\n\n");
        free(command);
        free(commandCopy);
        continue;
      }

      //check whether any node with this id already exists
      bool exists = false;
      while(token != NULL){
        if(mapFind(map, token) != NULL){
          printf("   [Warning] Node %s already exists in the graph.\n", token);
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

      token = strtok(commandCopy, " ");
      token = strtok(NULL, " ");

      printf("   [Success] Inserted node(s):");
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
      char* next = strtok(NULL, " ");

      if(token == NULL || token2 == NULL || sum == NULL || date == NULL || next != NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : n Ni Nj amount date\n\n");
      }
      else if(strcmp(token, token2) == 0){
        printf("   [Error] Transaction with the same origin and destination is not allowed.\n\n");
      }
      else{
        addEdge(graph, date, atoi(sum), token, token2, map);
        printf("   [Success] Transaction added: %s -> %s (Amount: %s, Date: %s)\n\n", token, token2, sum, date);
      }
    }


    //--------------------------------------------------------------- 3 --------------------------------------------------------
    else if(strcmp(token,"d") == 0 || strcmp(token, "delete") == 0){
      token = strtok(NULL, " ");
      
      if(token == NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : d Ni [Nj Nk ...]\n\n");
        free(command);
        free(commandCopy);
        continue;
      }

      //check whether every node id exists
      bool exists = false;
      while(token != NULL){
        if(mapFind(map, token) == NULL){
          printf("   [Warning] Node %s does not exist in the graph.\n", token);
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

      token = strtok(commandCopy, " ");
      token = strtok(NULL, " ");

      printf("   [Success] Deleted node(s):");
      while(token != NULL){
        printf(" %s", token);
        removeGraphNode(token, map, graph);
        token = strtok(NULL, " ");
      }
      printf("\n\n");

    }

    //--------------------------------------------------------------- 4 --------------------------------------------------------
    else if(strcmp(token, "l" ) == 0 || strcmp(token, "delete2") == 0){

      token = strtok(NULL, " ");
      char* token2 = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(next != NULL || token == NULL || token2 == NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : l Ni Nj\n\n");
      }
      else if(findEdge(token, token2, map) == NULL){
        printf("   [Warning] Edge between %s and %s not found.\n\n", token, token2);
      }
      else{
        removeEdge(token, token2, map);
        printf("   [Success] Transaction between %s and %s has been deleted.\n\n", token, token2);
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
        printf("   [Error] Invalid format.\n");
        printf("   Usage : m Ni Nj old_sum new_sum old_date new_date\n\n");
      }
      else if(modifyEdge(id1, id2, date, atoi(sum), date2, atoi(sum2), map) == 1){
        printf("   [Warning] The specified transaction was not found.\n\n" );
      }
      else{
        printf("   [Success] Transaction between %s and %s modified successfully.\n\n", id1, id2);
      }
    }

    //--------------------------------------------------------------- 6 --------------------------------------------------------

    else if(strcmp(token, "f") == 0 || strcmp(token, "find") == 0){
      token = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || next != NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : f Ni\n\n");
      }
      else if(mapFind(map, token) == NULL){
        printf("   [Warning] Node %s does not exist.\n\n", token);
      }
      else{
        printf("   [Result] Outgoing transactions from %s:\n", token);
        displayOutgoingEdges(token, map);
        printf("\n");
      }
    }

    //--------------------------------------------------------------- 7 --------------------------------------------------------

    else if(strcmp(token, "r") == 0 || strcmp(token, "receiving")==0){
      token = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || next != NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : r Ni\n\n");
      }
      else if(mapFind(map, token) == NULL){
        printf("   [Warning] Node %s does not exist.\n\n", token);
      }
      else{
        printf("   [Result] Incoming transactions to %s:\n", token);
        displayIncomingEdges(token, map);
        printf("\n");
      }      
    }

    //--------------------------------------------------------------- 8 --------------------------------------------------------
    else if(strcmp(token, "c") == 0 || strcmp(token, "circlefind") == 0){
      token = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || next != NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : c Ni\n\n");
      }
      else if(mapFind(map, token) == NULL){
        printf("   [Warning] Node %s does not exist.\n\n", token);
      }
      else{
        printf("   [Result] Simple cycles involving node %s:\n", token);
        findCircles(token, graph, map, 0, 0);
      }
    }

    //--------------------------------------------------------------- 9 --------------------------------------------------------
    
    else if(strcmp(token, "fi") == 0 || strcmp(token, "findcircles") == 0){
      token = strtok(NULL, " ");
      char* sum = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || sum == NULL || next != NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : fi Ni min_amount\n\n");
      }
      else if(mapFind(map, token) == NULL){
        printf("   [Warning] Node %s does not exist.\n\n", token);
      }
      else{
        printf("   [Result] Cycles involving %s with amount >= %s:\n", token, sum);
        findCircles(token, graph, map, atoi(sum), 1);
      }
    }


    //--------------------------------------------------------------- 11 --------------------------------------------------------
    else if(strcmp(token, "o") == 0 || strcmp(token, "connected") == 0){
      token = strtok(NULL, " ");
      char* id2 = strtok(NULL, " ");
      char* next = strtok(NULL, " ");

      if(token == NULL || id2 == NULL || next != NULL){
        printf("   [Error] Invalid format.\n");
        printf("   Usage : o Ni Nj\n\n");
      }
      else if(mapFind(map, token) == NULL){
        printf("   [Warning] Node %s does not exist.\n\n", token);
      }
      else if(mapFind(map, id2) == NULL){
        printf("   [Warning] Node %s does not exist.\n\n", id2);
      }
      else{
        printf("   [Result] Path from %s to %s:\n", token, id2);
        findPath(graph, token, id2, map);
      }
    }

    //--------------------------------------------------------------- HELP ------------------------------------------------------
    else if(strcmp(token, "h") == 0 || strcmp(token, "help") == 0){
      printMenu();
    }

    //--------------------------------------------------------------- 12 --------------------------------------------------------
    else if(strcmp(token, "e") == 0 || strcmp(token, "exit") == 0){
      printf("   [Info] Saving data and terminating the program. Goodbye!\n");
      exit = true;
    }
    else{
      printf("   [Error] Unrecognized command: %s\n", token);
      printf("   Type 'h' or 'help' to see the available commands.\n\n");
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