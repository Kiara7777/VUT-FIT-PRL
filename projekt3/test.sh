#!/bin/bash

#test parametru, $1 = string uzlu
if [ $# -ne 1 ]; then 
    echo "Spatne parametry. POUZITI: sh test.sh RETEZEC_UZLU"
	ok=false;
else
    nodes=$1;
	size=${#1}; 
	let "cpu = (2 * $size) - 2";
	ok=true;
fi;

if $ok; then

	#preklad cpp zdrojaku
	mpic++ --prefix /usr/local/share/OpenMPI -o pro pro.cpp

	#spusteni
	mpirun --prefix /usr/local/share/OpenMPI -np $cpu pro $nodes

	#uklid
	rm -f pro 	
fi;
