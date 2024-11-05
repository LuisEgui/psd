#!/bin/bash

# Clean previous files
make clean

# Compile saop library and make
soapcpp2 -b -c conecta4.h
make

# Start the server and redirect its output to a log file
./server 22500

# Start the clients and redirect their output to log files
for (( i=1; i<=4; i++ )); do
    echo "Player $i" | ./client http://localhost:22501 > "client_$i.log" 2>&1 &
done
