#!/bin/bash

for i in {a..z} {A..Z} {0..9}; do 
    for j in {a..z} {A..Z} {0..9}; do 
	for k in {z..z}; do 
	    echo -n "${i}*${j}h*e?l([${i}-${j}]|ot${j}h)${k}*ewre|"; 
	done
    done
done

echo -n "hesi..z|ma?rti*n|"
echo "theregexend"; 
