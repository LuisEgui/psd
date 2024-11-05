#!/bin/bash

# 20 clients
for i in {1..100}
do
    ./client http://localhost:22500 10 + 10 &
done