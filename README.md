# Graph Server with Active Objects

## Project Overview

This project implements a multi-threaded graph server using C++. It allows clients to perform various operations on a graph, including adding/removing vertices and edges, calculating Minimum Spanning Trees (MSTs), and computing graph metrics. The server utilizes an Active Object pattern for concurrent processing of requests.

## Features

- Graph operations: add/remove vertices and edges
- MST calculations using Prim's and Kruskal's algorithms
- Computation of graph metrics (total weight, longest distance, shortest distance, average distance)
- Multi-threaded server architecture using Active Objects
- Client-server communication over TCP/IP

## Project Structure


## Prerequisites

- C++17 compatible compiler
- Make
- Valgrind (for memory checks)
- LCOV (for code coverage)

## Compilation

To compile the project, run:

make

This will generate two executables: `server_exe` and `client_exe`.

## Running the Server

To start the server:

./server_exe

By default, the server listens on port 9036.

## Running the Client

To start the client:

./client_exe


The client will attempt to connect to localhost on port 9036.

## Testing

### Memory Check

To run memory checks using Valgrind:

./memcheck.sh

### Code Coverage

To generate a code coverage report:

./coverage.sh


The coverage report will be generated in the `coverage_report` directory.

## Usage

Once connected, the client can send various commands to the server:

- `add_vertex`: Add a new vertex to the graph
- `add_edge <v1> <v2> <w>`: Add an edge between vertices v1 and v2 with weight w
- `remove_vertex <v>`: Remove vertex v from the graph
- `remove_edge <v1> <v2>`: Remove the edge between vertices v1 and v2
- `calculate_mst <algo>`: Calculate the Minimum Spanning Tree using 'prim' or 'kruskal'
- `metrics_mst`: Get the MST and its metrics
- `help`: Show available commands
- `quit`: Exit the program

#########################################################################
FLOW OF THE PROGRAM
#########################################################################

+-------------+     HTTP     +------------------------+
|   Clients   | <----------> |         Server         |
+-------------+              +------------------------+
                             | - m_activeObjects: vec |
                             | - m_threadPool         |
                             +------------------------+
                             | + handleRequest()      |
                             | + processImage()       |
                             +------------------------+
                                         |
                                         | creates & manages
                                         v
                            +--------------------+  
                            |    ActiveObject    | 
                            +--------------------+    
                            | - m_pipeline       |      
                            | - m_stop: atomic   |       
                            +--------------------+      
                            | + start()          |       
                            | + stop()           |        
                            | - run()            |       
                            +--------------------+
                                        |
                                        | uses
                                        v
                            +--------------------+
                            |      Pipeline      |
                            +--------------------+
                            | - m_queue: SafeQueue|
                            +--------------------+
                            | + push()           |
                            | + pop()            |
                            | + tryPop()         |
                            +--------------------+
                                        |
                                        | uses
                                        v
                            +--------------------+
                            |     SafeQueue      |
                            +--------------------+
                            | - m_queue: queue   |
                            | - m_mutex: mutex   |
                            | - m_cond: cond_var |
                            +--------------------+
                            | + push()           |
                            | + pop()            |
                            | + tryPop()         |
                            +--------------------+
