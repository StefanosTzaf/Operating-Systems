# Miris: Dynamic Banking Graph & Flow Analyzer

## 🎯 Project Objectives & Scope
The goal of this project was to design and implement a custom, dynamically growing directed multi-graph in C to simulate, manage, and analyze banking transactions between users. 

**Key Requirements:**
* **No Static Limits:** The system must handle an arbitrary number of users and transactions without pre-allocating static arrays.
* **Fast Lookups:** Locating a specific user in the graph must be done in $O(1)$ time, bypassing the need to traverse the entire graph.
* **Pattern Detection:** The system must traverse the graph to detect specific financial patterns, such as money laundering loops (cycles) and multi-step money flows.
* **Memory Safety:** Strict memory management. All dynamically allocated memory must be properly freed upon exit, leaving zero memory leaks.

---

## 🏗️ Core Data Structures & Architecture

### 1. Generic Doubly Linked List (`List`)
I implemented a generic doubly linked list (`value = void*`) to serve as the foundational data structure. Using a doubly linked list allows for $O(1)$ node removal since we don't need to traverse the list to find the previous node.
* **Function Pointers:** Because the list holds generic data, it accepts function pointers (`CompareFunc` and `DestroyFunc`) from the user (e.g., the Graph) to know how to compare or efficiently delete specific payload types.
* **Opaque Pointers (Encapsulation):** The list structs are declared as incomplete types in the `.h` file. This prevents users of the list from directly mutating its internal pointers, enforcing interaction strictly through the provided API. This abstraction heavily simplified debugging.

### 2. Hash Map (`Map`)
To achieve the required $O(1)$ lookup time for user IDs to Graph Nodes, I implemented a Hash Table using separate chaining.
* **Collision Handling:** Collisions are resolved using linked lists (buckets). A solid hash function ensures these buckets remain small, maintaining an amortized $O(1)$ search time.
* **Capacity Heuristic:** The initial capacity is strategically set to `lines_in_file * 2 * 3` to comfortably accommodate the initial dataset and future inserts, keeping the load factor very low and avoiding the overhead of dynamic rehashing.

### 3. Directed Multi-Graph (`Graph`)
The graph stores users as nodes and their transactions as directed edges. 
* **Nodes:** Each node contains a string ID and two distinct lists of edges: `incoming` and `outgoing`.
* **Edges:** Each transaction is represented as an edge containing the date, the amount, and pointers to the source and destination graph nodes.
* **Shared Edge Pointers:** When an edge is created, a *pointer* to that single edge instance is added to both the source's outgoing list and the destination's incoming list. This avoids data duplication, saves memory, and ensures that any modification to the transaction is immediately reflected for both users.

---
## How to run the program
1. Compile the code using `make` in the terminal.
2. Run the program with `./build/miris -i input_file -o output_file`.

## ⚙️ CLI Commands & Internal Logic

> **Parsing Note:** User IDs are handled as strings. For commands accepting variable arguments (e.g., inserting/deleting multiple nodes at once), the input string is buffered. This allows the program to first validate the input and then cleanly parse the arguments using `strtok`.

* **`i` (Insert Node):** Checks if the node already exists via the Hash Map. If not, it inserts the node into both the Graph's list and the Map.
* **`n` (Insert Edge):** Adds a transaction. If the involved nodes don't exist, it creates them. It allocates the edge and links its pointer to the respective `incoming` and `outgoing` lists.
* **`d` (Delete Node):** Removes the node from the Hash Map to prevent future lookups, then deletes it from the Graph. The list's `DestroyFunc` handles the heavy lifting: it cleans up both edge lists. The edge's specific destroyer safely removes the edge pointer from the *other* node's list before freeing the edge memory.
* **`l` (Delete Edge):** Locates a specific transaction between two nodes and safely removes it from both their lists.
* **`m` (Modify Edge):** Finds a specific transaction in the source's outgoing list and updates its amount/date.
* **`f` / `r` (Find Flows):** Iterates through and prints a node's `outgoing` (Find) or `incoming` (Receiving) edge lists.
* **`c` (Find Simple Cycles):** Uses Depth-First Search (DFS) to find all simple cycles starting and ending at the target node. A list is used to track the current traversal path. When a cycle is detected, the path is printed. The path tracking list acts as a stack (pushing on visit, popping on backtrack) to explore all permutations.
* **`f` (Find Cycles with Threshold):** Similar to the DFS cycle detection above, but incorporates an early-exit condition: if a transaction amount is below a user-defined threshold, that branch is skipped.
* **`o` (Check Connection):** Uses DFS to determine if any path exists between Node A and Node B. A heap-allocated boolean flag is shared across the recursive calls to instantly signal success and halt further unnecessary traversal.
* **`e` (Exit):** Triggers a cascade of `free()` calls. The Graph destroys its node list, which in turn destroys all edge lists, edges, and node payloads. The Hash Map is completely deallocated.