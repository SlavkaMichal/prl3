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


declare -i proc=$(echo $1 | tr -cd ',' | wc -c)
proc=$((proc))
if [[ $proc -gt 20 ]]; then
    proc=20
fi

mpicxx -o vid vid.cpp

#echo "mpirun --prefix /usr/local/share/OpenMPI -np $proc vid $inp"
#echo $(mpirun --prefix /usr/local/share/OpenMPI -np $proc vid $inp < /dev/null )
mpirun --prefix /usr/local/share/OpenMPI -np $proc vid $inp < /dev/null

rm vid
