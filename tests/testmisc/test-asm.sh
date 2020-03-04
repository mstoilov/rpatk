#!/bin/bash

F=`find build/unix/ -name "asm*"`; 
for f in $F; 
do 
    echo "Running: $f"
    $f; 
done
