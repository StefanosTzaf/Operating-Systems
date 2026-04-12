# Linux Systems Programming & IPC

This repository contains three distinct projects exploring the internals of the Linux Operating System. The implementations focus heavily on low-level **C programming**, strict **dynamic memory management**, **Inter-Process Communication (IPC)**, and **concurrency control** using the POSIX API.

---

## 🗂️ Projects Overview

### [Project 1: Dynamic Banking Graph (`miris`)](./01_Miris_Dynamic_Banking_Graphs)
An introduction to advanced memory management and dynamic data structures in C.
* **Focus:** Dynamic memory allocation, custom Hash Maps (O(1) access), graph traversal algorithms (cycle detection, flow tracing).
* **Tech:** Pure C, Valgrind, Pointers, `Makefile`.

### [Project 2: Process Hierarchies & IPC (`lexan`)](./02_Lexan_Parallel_Text_Analysis_IPC)
A parallel lexical analysis tool utilizing a multi-level process hierarchy (Root -> Splitters -> Builders) to find the "Top-K" most popular words in a text file.
* **Focus:** Process creation (`fork`, `exec`), Inter-Process Communication using **Pipes** and asynchronous event handling via **Unix Signals** (`SIGUSR1`, `SIGUSR2`).

### [Project 3: Concurrency & Synchronization (`Nemea Bar`)](./03_NemeaBar_Concurrent_Simulation_Sync)
A complex concurrent system solving a variation of the classical synchronization problems. It coordinates multiple independent processes (visitors, receptionist, monitor) interacting in a simulated bar environment.
* **Focus:** **POSIX Semaphores** (P/V operations), Mutual Exclusion, and Deadlock/Starvation avoidance.

---

## 🚀 How to Run
Each project is contained within its respective directory. Navigate to a specific project folder to find its dedicated `Makefile` and detailed execution instructions in its local `README.md`.

## 🎓 Academic Context
These projects were developed as part of the **"Operating Systems"** course at the Department of Informatics and Telecommunications, National and Kapodistrian University of Athens (NKUA).