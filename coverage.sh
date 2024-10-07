#!/bin/bash

PORT=9035
MAX_ATTEMPTS=5

# Function to check if port is in use
is_port_in_use() {
    netstat -tuln | grep :$PORT > /dev/null
}

# Function to find and kill process using the port
kill_process_on_port() {
    local pid=$(lsof -t -i:$PORT)
    if [ ! -z "$pid" ]; then
        echo "Killing process $pid using port $PORT"
        kill -9 $pid
    fi
}

# Try to free up the port
for i in $(seq 1 $MAX_ATTEMPTS); do
    if is_port_in_use; then
        echo "Port $PORT is in use. Attempting to free it up (Attempt $i/$MAX_ATTEMPTS)"
        kill_process_on_port
        sleep 2
    else
        break
    fi
done

# If port is still in use, increment it
if is_port_in_use; then
    echo "Port $PORT is still in use. Trying next available port."
    while is_port_in_use; do
        PORT=$((PORT+1))
    done
    echo "Using port $PORT"
fi

# Update server code with new port
sed -i "s/Server server([0-9]\+)/Server server($PORT)/" server/src/main.cpp

# Compile the program with coverage flags
if ! make CXXFLAGS="-std=c++17 -Wall -Wextra -pthread -Icommon -fprofile-arcs -ftest-coverage --coverage"; then
    echo "Compilation failed. Please fix the errors and try again."
    exit 1
fi

# Check if server_exe exists
if [ ! -f "./server_exe" ]; then
    echo "server_exe not found. Compilation may have failed."
    exit 1
fi

# Function to run server and restart if it crashes
run_server() {
    while true; do
        ./server_exe &
        SERVER_PID=$!
        wait $SERVER_PID
        if [ $? -eq 0 ]; then
            break
        fi
        echo "Server crashed. Restarting..."
        sleep 1
    done
}

# Run server in background
run_server &

# Wait for server to start
sleep 2

# Function to send a command to the server
send_command() {
    echo "Sending command: $1"
    echo "$1" | nc -w 1 localhost $PORT
    sleep 0.5
}

# Run test commands
commands=(
    "add_vertex"
    "add_vertex"
    "add_vertex"
    "add_edge 0 1 5"
    "add_edge 1 2 3"
    "add_edge 0 2 7"
    "get_vertex_count"
    "get_edge_count"
    "get_adjacent_vertices 1"
    "get_edge_weight 0 1"
    "calculate_mst prim"
    "calculate_mst kruskal"
    "is_connected"
    "remove_edge 0 1"
    "get_edge_count"
    "calculate_mst prim"
    "remove_vertex 2"
    "get_vertex_count"
    "get_edge_count"
    "is_connected"
    "add_vertex"
    "add_edge 0 3 4"
    "add_edge 1 3 6"
    "calculate_mst prim"
    "calculate_mst kruskal"
    "is_connected"
    "change_weight 0 3 2"
    "get_edge_weight 0 3"
    "calculate_mst prim"
    "calculate_mst kruskal"
    "add_edge 0 10 5"
    "remove_edge 0 10"
    "get_edge_weight 0 10"
    "calculate_mst invalid_algorithm"
    "get_adjacent_vertices 10"
    "remove_vertex 10"
    "quit"
)

for cmd in "${commands[@]}"; do
    send_command "$cmd"
    if ! kill -0 $SERVER_PID 2>/dev/null; then
        echo "Server crashed. Restarting..."
        run_server &
        sleep 2
    fi
done

# Kill the server
echo "Stopping server..."
kill -SIGINT $SERVER_PID
sleep 2

# If the server is still running, force kill it
if kill -0 $SERVER_PID 2>/dev/null; then
    echo "Server did not shut down gracefully. Force killing..."
    kill -9 $SERVER_PID
fi

# Wait a moment to ensure all files are written
sleep 2

# Remove placeholder .gcda files
find common client/src server/src -name "*.gcda" -type f -size 0c -delete

# Generate coverage report
echo "Generating coverage report..."
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_report

echo "Code coverage report generated in coverage_report/index.html"

