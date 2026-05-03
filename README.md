## Turtle DB 🐢

A simple, educational relational database management system built from scratch in modern C++ (C++20). The primary goal of this project is to explore and implement the core components of a relational database, focusing on the storage engine, query execution, type system, and indexing.

# Features

- [x] **Disk Manager**: A low-level manager for reading and writing 4096-byte pages to and from files on disk.
- [x] **LRU Replacer**: A page replacement policy implementation to decide which pages to evict from memory.
- [x] **Buffer Pool Manager**: An in-memory cache for database pages, reducing the need for slow disk I/O and handling dirty page flushes.
- [x] **Catalog**: A system catalog to track metadata about databases, schemas, tables, and columns.
- [x] **Slotted Page Architecture**: A robust page layout for storing variable-length tuples safely in memory.
- [x] **Table Heap & Iterator**: A linked-list architecture allowing tables to grow infinitely across multiple pages, with seamless traversal.
- [x] **Type System**: A dynamic, extensible type system (currently featuring `IntegerType`) supporting arithmetic, comparison, and safe binary serialization.
- [x] **Execution Engine (Volcano Model)**: A functional pipeline currently supporting `SeqScanExecutor` to retrieve tuples from disk to memory programmatically.
- [ ] SQL Parser: A parser to transform SQL strings into an Abstract Syntax Tree (AST).
- [ ] B+ Tree Indexing: A B+ Tree implementation for efficient data indexing and fast point-lookups.
- [ ] Concurrency Control: Transaction management using strict 2PL locking protocols.

# Architecture

The system is designed with a classic layered architecture, ensuring a clean separation of concerns between different database components.

1. **SQL Layer (Pending)**: The highest level, responsible for parsing, planning, and optimizing SQL queries.
2. **Execution Engine (In Progress)**: Executes query plans by requesting data from the storage layer using a Volcano-style iterator model (`init()`, `next()`).
3. **Type System (Active)**: Handles dynamic type casting, null-bitmaps, and binary serialization logic decoupled from the physical page layout.
4. **Storage Engine (Active)**: The core of the database, responsible for managing how data is stored in memory and on disk.
   1. **Table Heap**: Chains memory frames together logically to represent a standard SQL Table.
   2. **Buffer Pool Manager**: Manages a pool of in-memory page frames. It fetches pages from disk into memory when needed, handles pinning/unpinning, and manages eviction.
   3. **Disk Manager**: The lowest level of the storage engine, responsible for the actual binary I/O operations with the database `.db` files.

# Project Roadmap

1. [x] **Implement a Catalog**: Create a system catalog to store metadata about tables, columns, and schemas.
2. [x] **Build a Heap Table Manager**: Implement a slotted page layout for storing tuples and chain them via a `TableHeap`.
3. [x] **Build an Execution Engine**: Implement a Volcano-style execution engine with basic operators (e.g., Sequential Scan).
4. [ ] **Develop a B+ Tree Index**: Implement a disk-backed B+ Tree (`BPlusTreePage`, `BPlusTreeInternalPage`, `BPlusTreeLeafPage`) to replace slow sequential scans.
5. [ ] **Implement Insert/Update Executors**: Allow the execution engine to mutate data (currently handled manually).
6. [ ] **Create a SQL Parser**: Use a tool like Flex/Bison (or recursive descent) to build a parser for basic SQL commands (SELECT, INSERT, CREATE).

# License

This project is licensed under the MIT License.
