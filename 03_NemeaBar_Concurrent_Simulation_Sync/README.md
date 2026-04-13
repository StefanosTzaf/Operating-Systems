# 🍻 Bar / Restaurant IPC Simulation

## 🎯 Project Overview & Objectives
The goal of this project is to simulate the realistic daily operations of a bar/restaurant using **Inter-Process Communication (IPC)** mechanisms in C. 

In this simulation, **Visitors** arrive at the bar (either automatically spawned at startup or arriving dynamically via the command line). They check for available tables, wait in line if the bar is full, take a seat, and place **random food/drink orders from a menu**. A **Receptionist** (acting as the bartender/waiter) receives these orders, spends time preparing them, and serves the visitors so they can eat for a random amount of time before leaving. 

Behind the scenes, this highly concurrent system models the interactions between these distinct processes using **POSIX Shared Memory** and **Semaphores** for strict synchronization, mutual exclusion, and deadlock prevention.

**Key Requirements & Policies implemented:**
* **Concurrent Entities:** The system consists of distinct processes for the Receptionist, Visitors, an Initializer, a Monitor, and a Closer.
* **Resource Management:** The bar has tables with 4 chairs each (12 chairs in total). Visitors must wait if the bar is full. A table freed by 4 visitors becomes available for new ones.
* **First-Come, First-Served (FCFS):** Strict FCFS policies must be enforced both for visitors waiting to take a seat and for visitors placing orders.
* **Extensive Logging:** Every significant action (arrival, waiting, sitting, ordering, being served, and leaving) is recorded in a shared text log file to track the lifecycle of each visitor.
* **Graceful Termination:** The bar must close gracefully, allowing already seated or waiting visitors to finish their food and leave without forcefully killing processes.

---

## ⚠️ Important Warning
> **WARNING:** Programs are terminated **ONLY** by calling `./closer -s <sharedMemoryName>` from another terminal. Do not use `Ctrl+C` or kill the processes forcefully, as this will leave shared memory and semaphores unlinked. More details in the "Closing the Bar" section below.

---

## 🚀 Command Line Execution

POSIX shared memory instructions are used (as mentioned in Piazza), so we provide a `sharedMemoryName` instead of a traditional `shmid`.

### 1. Initializer
`./initializer -d <orderTime> -r <restTime> -s <sharedMemoryName> -l <logFileName.txt>`
* `orderTime`: Parameter for the receptionist's preparation time.
* `restTime`: Parameter for the visitors' eating time.
* `logFileName.txt`: The file where logging facts will be stored (created if it does not exist).
* *Note:* A predefined number of visitors (defined in the code) are created by default. Visitors can also be created manually via another terminal.

### 2. Receptionist
`./receptionist -d <orderTime> -s <sharedMemoryName> -l <logFileName.txt>`
* Parameters must match the initializer!
* *Implementation Note:* For more convenience, the receptionist is created with `fork` and `exec*` **inside** the initializer automatically.

### 3. Visitor
`./visitor -d <restTime> -s <sharedMemoryName> -l <logFileName.txt>`
* Parameters must match the initializer.
* Some visitors are created by the initializer, but you are free to spawn more dynamically via the command line to simulate busy hours.

### 4. Monitor
`./monitor -s <sharedMemoryName>`
* Prints the current state of shared memory. Only called via command line.

---

## 🧠 Shared Memory Architecture & Defines

The main shared memory struct consists of sub-structs to enhance modularity and maintain clean code.

### 1. Waiting Queue (FCFS Policy)
* **Circular Waiting Buffer:** Ensures FCFS policy for the first `MAX_VISITORS` (defined in `shm_structs.h` and easily modifiable). 
* **Semaphores:** One semaphore for **each** position of the buffer ensures that only one process is suspended there. (If we just used a single semaphore for the waiting room, processes would wake up randomly, breaking FCFS).
* **Exceeding Semaphore:** Because the shared memory segment must be defined dynamically before execution, we cannot have an infinite FCFS buffer. There is an extra semaphore for exceeding visitors (initialized to `MAX_VISITORS`). If the FCFS buffer is full, excess visitors are suspended here (without strict FCFS fairness).

### 2. Orders Queue
* **Orders Circular Buffer:** Ensures the first customer who ordered is served first. It has 12 positions (matching the number of chairs, since only seated visitors can order).
* A `menuOrder` sub-struct indicates what every order consists of (simulating the randomized menu choices of each visitor). 

### 3. State & Synchronization
* **Statistics Struct:** Keeps track of bar metrics (total visitors, orders served, etc.).
* **Tables Array:** An array of `table` structs storing the state of tables (occupied or not, specific chairs).
* **General Semaphores:** One for Mutual Exclusion (Mutex) and one for the Receptionist (suspended here when there is no work).
* **Closing Flag:** A boolean variable indicating if the bar is closing.

---

## 🔄 Process Workflow (Function of the Bar)

### The Initializer & Receptionist
Everything starts with the initializer call. The Receptionist and some initial Visitors are made and executed. 
* The Receptionist is initially suspended in its semaphore since there is no work. 
* Whenever awoken, it checks if the bar is closing. If not, it prepares orders using the FCFS policy.
* It updates statistics, sleeps for a random time (simulating order preparation based on `orderTime`), and then wakes up the specific visitor waiting in the chair whose order was just prepared.

### The Visitor
1.  **Arrival:** Checks if the bar is closing. If yes, it does not enter.
2.  **Queueing:** Decreases the exceeding buffer semaphore. If the FCFS buffer is full, it is suspended there.
3.  **Seating:** Checks if the buffer is empty AND a table is available. If not, it is suspended in the buffer.
4.  **Ordering:** Takes a seat, **generates a random food/drink order from the menu**, wakes up the Receptionist, and waits suspended on its specific chair semaphore.
5.  **Eating & Leaving:** Woken up by the Receptionist, eats for a random time (simulated via `restTime`), logs its departure, and leaves.
6.  **IMPORTANT (Deadlock Prevention):** Before leaving, if the visitor was the **last one of a full table**, they wake up 4 visitors waiting in the buffer to take a seat. Additionally, if they are the **last one in the entire bar** and the bar is closing, they wake up the Receptionist to trigger the final shutdown.

### Closing the Bar
The bar closes only when the `closer` process is called. 
* When the closing flag is set to `1`, new visitors can no longer enter.
* Visitors already inside (eating, waiting for an order, or waiting to sit) are **NOT** terminated by force; they finish their lifecycle properly.

There are two cases when closing:
1.  **No visitors inside:** The `closer` process MUST manually wake up the Receptionist to terminate.
2.  **Visitors inside:** The very last visitor leaving the bar wakes up the Receptionist to terminate.

---

## 📝 Clarifications & Notes

* **Logging & Tracking:** The log file (`log.txt`) is highly detailed and used by all processes. It acts as the "history book" of the bar, recording exact moments of arrivals, seatings, orders, and departures. When manually creating a visitor via the CLI, ensure you provide the exact same log file name. 
* **Table Occupation Rules:** If a visitor has sat down and the table has not yet been occupied by 4 people, and the visitor leaves, that chair can be occupied by another visitor immediately. As required, a table is only entirely "freed" (reset for a brand new group) when it was completely full (4 visitors) and all 4 have left.
* **Time Metrics:** Average times are not stored directly in shared memory because they can be easily calculated on-the-fly (Total Time / Total Visitors).
* **Stay Time Definition:** The "Average Stay Time" does **NOT** include the time a visitor spends waiting to take a seat. It strictly measures the time from when they sit down until they leave the bar.