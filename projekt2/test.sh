#!/bin/bash

#test parametru, $1 = pocet cisel (pro soubor numbers), $2 = pocet procesoru co provadi vypocet
if [ $# -ne 2 ]; then 
    echo "Spatne parametry. POUZITI: sh test.sh POCET_HODNOT POCET_PROCESORU"
	ok=false;
else
    numbers=$1;
	cpu=$2;
	ok=true;
fi;

if $ok; then

	#preklad cpp zdrojaku
	mpic++ --prefix /usr/local/share/OpenMPI -o mss mss.cpp


	#vyrobeni souboru s random cisly
	dd if=/dev/random bs=1 count=$numbers of=numbers

	#spusteni
	mpirun --prefix /usr/local/share/OpenMPI -np $cpu mss

	#uklid
	rm -f mss numbers	
fi;
