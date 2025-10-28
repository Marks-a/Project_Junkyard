
g++ -o server server.cpp
g++ -o cli cli.cpp

./server &
SERVER_PID=$!

trap "echo 'Stopping server...'; kill $SERVER_PID; exit" INT

for i in {1..5}; do
    echo "=============================="
    ./cli
    sleep 3
done

kill $SERVER_PID
