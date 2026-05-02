## Turtle DB 🐢

A simple, educational database management system build from scratch in modern C++ (C++17). The primary goal of this project is to explore and implement the core components of a relational database, focusing on the storage engine, query processing, and indexing.

# Features

- [x] Disk manager: A low-level manager for reading and writing pages to and from files on disk.
- [x] LRU Replacer: A page replacement policy implementation to decide which pages to evict from memory.
- [x] Buffer Pool Manager: An in-memory cache for database pages, recuding the need for slow disk I/O.
- [ ] SQL Parser: A parser to transform SQL strings into Abstract Syntax Tree (AST).
- [ ] Execution Engine: An engine to execute query plans using a Volcano-style model.
- [ ] B+ Tree Indexing: A B+ Tree implementation for efficent data indexing and retrieval.
- [ ] Concurrency Control: Transaction management using locking protocols.

# Architecture

The system is designed with a classic layered architecture, ensuring a clean separation of concerns between different database components.

1. SQL Layer: The highest level, responsible for parsing, planning, and optimizing SQL queries.
2. Execution Engine: Executes the query plan by requesting data from the layer below.
3. Storage Engine: The core of the database, responsbile for managing how data is stored in memory and on disk.
   1. Buffer Pool Manager: Manages a pool of in-memory page frames. It fetches pages from disk into memory when needed and handles eviction.
   2. Disk Manager: The lowest level of the storage engine, responsible for the actual I/O operations with the database files.

# Project Roadmap

1. Implement a Catalog: Create a system catalog to store metadata about tables, columns, and schemas.
2. Build a Heap Table Manager: Implement a simple page-based layout for storing tuples (records).
3. Develop a B+ Tree Index: Implement a B+ Tree for primary and secondary indexes.
4. Create a SQL Parser: Use a tool like Flex/Bison to build a parser for basic SQL commands (SELECT, INSERT, CREATE).
5. Build an Execution Engine: Implement a Volcano-style execution engine with basic operators (e.g., Sequential Scan, Insert).

# License

This project is licensed under the MIT License.
