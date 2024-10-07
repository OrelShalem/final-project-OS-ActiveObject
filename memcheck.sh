#!/bin/bash

# Compile the program
if ! make; then
    echo "Compilation failed. Please fix the errors and try again."
    exit 1
fi

# Check if server_exe exists
if [ ! -f "./server_exe" ]; then
    echo "server_exe not found. Compilation may have failed."
    exit 1
fi

# Run with Valgrind's memcheck
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=memcheck_report.txt ./server_exe &
MEMCHECK_PID=$!

# Wait for a bit to allow the server to start
sleep 2

# Function to send a command to the server and get the response
send_command() {
    echo "Sending command: $1"
    response=$(echo "$1" | nc -w 2 localhost 9035)  # Ensure this port matches your server
    echo "Response: $response"
    sleep 1  # Increased wait time between commands
}

# Run test commands
send_command "add_vertex"
send_command "add_vertex"
send_command "add_edge 0 1"
send_command "add_vertex"
send_command "add_edge 1 2"
send_command "remove_edge 0 1"
send_command "remove_vertex 1"
send_command "get_vertex_edges 0"
send_command "get_all_vertices"
send_command "clear_graph"
send_command "add_vertex"
send_command "add_vertex"
send_command "add_edge 0 1"
send_command "is_connected"
send_command "get_shortest_path 0 1"
send_command "invalid_command"  # Test error handling

# Allow some time for the server to process all commands
sleep 2

# Kill the memcheck process
kill $MEMCHECK_PID

echo "Memcheck report generated in memcheck_report.txt"

# Analyze the Memcheck report
if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" memcheck_report.txt; then
    echo "No Memcheck errors detected."
else
    echo "Memcheck detected errors. Please check memcheck_report.txt for details."
    grep -n "ERROR SUMMARY:" memcheck_report.txt
fi

# Check for memory leaks
if grep -q "definitely lost:" memcheck_report.txt; then
    echo "Warning: Memory leaks detected. Please review memcheck_report.txt."
    grep -n "definitely lost:" memcheck_report.txt
fi

# Check for uninitialized values
if grep -q "Conditional jump or move depends on uninitialised value(s)" memcheck_report.txt; then
    echo "Warning: Use of uninitialized values detected. Please review memcheck_report.txt."
fi

# Check for invalid memory accesses
if grep -q "Invalid read" memcheck_report.txt || grep -q "Invalid write" memcheck_report.txt; then
    echo "Warning: Invalid memory accesses detected. Please review memcheck_report.txt."
fi