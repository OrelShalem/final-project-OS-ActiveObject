#!/bin/bash

# Compile the program
make

# Run with Valgrind's helgrind
valgrind --tool=helgrind --log-file=helgrind_report.txt ./server_exe &
HELGRIND_PID=$!

# Wait for a bit to allow the server to start
sleep 2

# Function to send a command to the server and get the response
send_command() {
    echo "Sending command: $1"
    response=$(echo "$1" | nc -w 2 localhost 9035)  # Increased timeout to 2 seconds
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

# Kill the helgrind process
kill $HELGRIND_PID

echo "Helgrind report generated in helgrind_report.txt"

# Analyze the Helgrind report
if grep -q "ERROR SUMMARY: 0 errors from 0 contexts" helgrind_report.txt; then
    echo "No Helgrind errors detected."
else
    echo "Helgrind detected errors. Please check helgrind_report.txt for details."
    grep -n "ERROR SUMMARY:" helgrind_report.txt
fi

# Check for possible deadlocks
if grep -q "Possible deadlock" helgrind_report.txt; then
    echo "Warning: Possible deadlock detected. Please review helgrind_report.txt."
fi

# Check for data races
if grep -q "Possible data race" helgrind_report.txt; then
    echo "Warning: Possible data race detected. Please review helgrind_report.txt."
fi