# Lexan: Parallel Lexical Analyzer

## 🎯 Project Objective
The goal of this project is to implement a highly parallelized Lexical Analyzer using a multi-process architecture in C. The system reads a large text file, tokenizes it into words, calculates the absolute and relative frequency of each word, and outputs the "Top-K" most frequent words. 

To achieve high performance, the workload is distributed across a 3-tier process hierarchy: a **Root** process, multiple **Splitters**, and multiple **Builders**. The project heavily utilizes Inter-Process Communication (IPC) via unnamed pipes, process creation (`fork`, `exec`), signal handling (`SIGUSR1`, `SIGUSR2`), and custom data structures (Hash Maps and Binary Search Trees) to efficiently aggregate and sort massive amounts of text data.

---

## 🏗️ Files and Modularity

The codebase is highly modular. The system is divided into three distinct executable programs representing the three types of processes in our hierarchy. For every core process, there is a dedicated `utils` file containing its specific helper functions.

* **`root.c` / `lexan`:** The main orchestrator process. Creates pipes, spawns children, aggregates final results, and handles user input.
* **`splitter.c`:** Responsible for reading a specific chunk of the input file, parsing words, and distributing them to the Builders.
* **`builder.c`:** Responsible for receiving words via pipes, counting their occurrences using a Hash Table, and sending the aggregated data back to the Root.

---

## ⚙️ Process Roles & Workflow

### 1. Lexan (Root Process)
The Root process initializes the environment and orchestrates the workflow:
* **Workload Division:** Parses command-line arguments and scans the input file to count the total lines. It divides the lines equally among the Splitters (handling imperfect divisions by assigning remaining lines to the last Splitter).
* **Offset Calculation:** It calculates the exact byte offset where each Splitter should start reading. This ensures Splitters use `lseek` to jump directly to their assigned chunk, preventing redundant file reads.
* **IPC Setup:** Creates $N$ pipes between Splitters and Builders (where $N$ is the number of Builders) and a single shared pipe for Builders to send data back to the Root.
* **Execution:** Forks and executes (`exec*`) the Splitters and Builders. It strictly closes all unused file descriptors in both parent and child processes to prevent deadlocks.
* **Concurrent Aggregation:** As Builders finish and write to the final pipe, the Root reads the data and inserts it into a **Set (implemented as a Binary Search Tree)**. This allows the Root to sort the data concurrently while Builders are still working.
* **Output:** Once the `read()` syscall unblocks (indicating all Builder write-ends are closed), the Root uses `setLast` and `nodeFindPrevious` to effortlessly print the Top-K words and their relative frequencies to the output file.

### 2. Splitters
Spawned by the Root, Splitters act as the text parsers:
* **Targeted Reading:** Each Splitter jumps to its predefined byte offset and reads its assigned lines.
* **Tokenization & Filtering:** Words are separated by whitespace or punctuation. Numbers are ignored. Single-letter tokens (e.g., the 's' or 't' from "Bob's" or "isn't") are dropped. Everything is converted to lowercase (e.g., "Word" and "WoRd" are treated as identical).
* **Exclusion List:** Words are checked against an Exclusion Hash Table in $O(1)$ time. If a word is on the exclusion list, it is dropped.
* **Routing:** Valid words are hashed, and based on the modulo of the hash, sent to a specific Builder via the corresponding pipe.

### 3. Builders
Builders act as the aggregators:
* **Local Counting:** They continuously read words from their incoming pipes and store them in a local Hash Table. (The Hash Table's capacity is dynamically initialized based on an approximation of the expected word count, multiplied by 2 to keep the load factor low and prevent collisions).
* **Aggregation:** If a word arrives for the first time, it is inserted with an occurrence counter of 1. If it already exists, its counter is simply incremented.
* **Flushing:** Once all Splitters close their pipes, the `read()` call in the Builder returns 0. The Builder then iterates through its Hash Table and writes the aggregated data (Word + Occurrence Counter) to the Root pipe using a specific string format.

---

## 📡 Signals & IPC Safety
* **Signal Handling:** Two global variables are defined to track the number of received `SIGUSR1` (from Splitters) and `SIGUSR2` (from Builders) signals. The handler functions are defined in `rootUtils.c`.
* **Syscall Interruption Safety:** The `struct sigaction` is used with the `SA_RESTART` flag. This ensures that blocking system calls (like `read` or `wait`) are automatically restarted if interrupted by an incoming signal, preventing premature failures.

## ⏱️ Performance Metrics
Upon successful execution, the Root process calculates and prints to the standard output:
1. The total number of `SIGUSR1` and `SIGUSR2` signals received.
2. The **Real Time** (wall-clock time) vs. the **CPU Time** utilized by the program, demonstrating the efficiency and overhead of the parallelized architecture.