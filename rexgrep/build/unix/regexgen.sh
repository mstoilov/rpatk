#!/bin/bash

for i in {a..z} ; do 
    for j in {a..z} {A..Z}; do 
	echo -n "${i}*${j}h*e?l${i}othere|"; 
    done
done

echo -n "hesi..|martin|lesssthan.*there|hello.*|mello.*|fori|morei|elsi|whilei|doio|l*(i(n.*)*.)+ux|ponty|montyie|smuzi|lussi.*|"
echo "theregexend"; 
