#!/bin/bash

F=`find unix/ -name "asm*"`; 
for f in $F; 
do 
    echo "Running: $f"
    $f; 
done
