#!/bin/bash

# Usage: ./run.sh program1.cpp
# Compiles the given .cpp file (plus glad.c) and runs it.

if [ -z "$1" ]; then
    echo "Usage: ./run.sh <file.cpp>"
    exit 1
fi

SRC_FILE="src/$1"

if [ ! -f "$SRC_FILE" ]; then
    echo "Error: $SRC_FILE not found"
    exit 1
fi

g++ "$SRC_FILE" src/glad.c -o app -I include -lglfw -lGL -ldl && ./app
