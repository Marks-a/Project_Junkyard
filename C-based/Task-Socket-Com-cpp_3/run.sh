#!/bin/bash
set -e 

g++ -o server server.cpp
g++ -o cli cli.cpp


echo "Starting server..."
(./server | sed 's/^/[SERVER] /') &
SERVER_PID=$!
echo "Server started with PID: $SERVER_PID"

cleanup() {
    echo -e "\nStopping server (PID: $SERVER_PID)..."
    kill $SERVER_PID 2>/dev/null || true
    wait $SERVER_PID 2>/dev/null || true
    echo "Server stopped. Exiting."
    exit 0
}
trap cleanup INT TERM

sleep 2

for i in {1..5}; do
    echo "---- Running client iteration $i ----"
    ./cli | sed 's/^/[CLI] /'
    sleep 3
done

cleanup
