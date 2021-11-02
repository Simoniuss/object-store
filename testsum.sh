#!/bin/sh

cl_tot=0;
i=0
while read line
do
	if [ line = "Start test..." ]
	then
		continue;
	fi
	read -ra name <<<"$line"
	#name[0]="Utente" name[1]=nome utente name[2]="Test" name[3]=num test
	#name[4]=op totali name[5]=op successo name[6]=op fallite
	((cl_tot++))
	if [ "${name[3]}" == "1" ]
	then
		if [ ${name[6]} -gt 0 ]
		then
			fail_usr[i]=${name[1]}
			fail_bat[i]=${name[3]}
			n_fail[i]=${name[6]}
			((i++))
		fi
	elif [ "${name[3]}" == "2" ]
	then
		if [ ${name[6]} -gt 0 ]
		then
			fail_usr[i]=${name[1]}
			fail_bat[i]=${name[3]}
			n_fail[i]=${name[6]}
			((i++))
		fi
	elif [ "${name[3]}" == "3" ]
	then
		if [ ${name[6]} -gt 0 ]
		then
			fail_usr[i]=${name[1]}
			fail_bat[i]=${name[3]}
			n_fail[i]=${name[6]}
			((i++))
		fi
	fi
done < "testout.log"
echo Clienti lanciati ${cl_tot}
for (( j=0; j<$i; j++ ))
do
	echo Utente ${fail_usr[$j]} ha fallito ${n_fail[$j]} per la batteria ${fail_bat[$j]}
done
pkill -SIGUSR1 -f ./server_objstr
pkill -SIGINT -f ./server_objstr





