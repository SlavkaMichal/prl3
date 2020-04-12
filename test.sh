#!/bin/bash

if [[ $# != 1 ]]; then
    echo "Usage: $0 input"
    exit 1
fi

inp=""
if [[ $1 =~ ^[0-9,] ]]; then
    inp=$(echo $1 |sed 's/,/ /g')
else
    echo "Incorrect input format"
    exit 1
fi

echo $inp

declare -i proc=$(echo $1 | tr -cd ',' | wc -c)
proc=$((proc))
echo "$proc"
if [[ $proc -gt 25 ]]; then
    proc=25
fi
proc=2

mpicxx -o vid vid.cpp

mpirun --prefix /usr/local/share/OpenMPI -np $proc vid $inp

rm vid
