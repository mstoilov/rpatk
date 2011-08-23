#!/bin/bash

F=`find build/ -name "asm*"`; 
for f in $F; 
do 
    echo "Running: $f"
    $f; 
done
