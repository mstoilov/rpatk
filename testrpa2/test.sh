#!/bin/bash

F=`find build/ -name "rpavm*"`; 
for f in $F; 
do 
    $f; 
done

F=`find build/ -name "rpacompiler*"`; 
for f in $F; 
do 
    $f; 
done
