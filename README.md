# Time-Series-Data-Engine-
# Global Country Time Series Analyzer

A multi-stage C++ system built to efficiently store, analyze, and query over 200+ global country time series datasets using advanced data structures and algorithms. Designed and implemented as part of the ECE 250 course at the University of Waterloo.

## 🔧 Project Overview

This project simulates a robust backend system for storing country-specific numerical time series data and provides efficient query capabilities through evolving data structures:

- ✅ **Linked List** (Project 1): Initial linear-time implementation for basic data storage.
- 🌲 **N-ary Tree** (Project 3): Enables logarithmic-time (O(log N)) hierarchical queries for range-based mean comparisons.
- ⚡ **Double-Hashed Table** (Project 4): Achieves constant-time (O(1)) data lookups using open addressing and deletion markers.
- 🌐 **Graph Module** (Project 5): Supports dynamic country relationship modeling using BFS/DFS for adjacency/path queries.

## 📈 Key Features

- 🧠 **O(log N) queries** using balanced n-ary tree traversal.
- ⚙️ **512-slot double-hashed hash table** for efficient insertions, deletions, and lookups.
- 🌍 **Graph search algorithms (BFS/DFS)** for country-to-country relationship queries.
- 🧮 **Mean-based range queries** for comparative data analysis.
- ⌨️ **Command-based I/O system** for live interaction and testing.

## 🚀 How to Run

```bash
g++ -std=c++11 main.cpp -o country_analyzer
./country_analyzer < input.txt
