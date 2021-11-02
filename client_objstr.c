#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <obj_str.h>
#include <my_utils.h>

// Il numero di oggetti da creare.
#define N 20
// Stringa da inserire nei file.
#define str "abcd"

/*
** Struttura che contiene l'esito dei test effettuati.
** -op_eff: operazioni effettuate.
** -op_succ: operazioni completate con successo.
** -op_fall: operazioni fallite.
** -op_conf: operazioni di confronto con successo.
** -op_confall: operazioni di confronto fallite.
*/
typedef struct stat_test {
	int op_eff;
	int op_succ;
	int op_fall;
	int op_conf;
	int op_confall; 
}s_t;

/*
** Questa funzione crea N file con dentro "str" ripetute i*400 volte,
** dove i<=N.
** @param: -s: è la struttura dati utilizzata per l'aggiornamento delle stat.
** @return: la funzione ritorna 1 se è stata completata con successo, 0 se con errore.
*/
int store(s_t* s) {
	for(int i=1; i<=N; i++) {
		// Nome del file=i.
		char* filename=(char*)malloc(10*sizeof(char));
		if(check_eq(filename,NULL,"malloc filename store"))
			return 0;
		memset(filename,'\0',10);
		sprintf(filename,"%d",i);
		// Blocco dati da salvare.
		int n_str=i*400;
		char* msg=(char*)malloc(n_str*strlen(str)*sizeof(char));
		if(check_eq(msg,NULL,"malloc msg store")) {
			free(filename);
			return 0;
		}
		memset(msg,'\0',n_str*strlen(str)*sizeof(char));
		for(int j=0; j<n_str; j++)
			strncat(msg,str,strlen(str));
		s->op_eff++;
		if(!os_store(filename,(void*)msg,n_str*strlen(str)*sizeof(char)))
			s->op_fall++;
		else
			s->op_succ++;
		free(msg);
		free(filename);
	}
	return 1;
}

/*
** Questa funzione restituisce il contenuto del file "i" e lo confronta
** con una stringa formata da "str" ripetuta i*400 volte.
** @param: -s: è la struttura dati utilizzata per l'aggiornamento delle stat.
** @return: la funzione ritorna 1 se è stata completata con successo, 0 se con errore.
*/
int retrieve(s_t* s) {
	for(int i=1; i<=N; i++) {
		// Nome del file=i.
		char* filename=(char*)malloc(10*sizeof(char));
		if(check_eq(filename,NULL,"malloc filename retrieve"))
			return 0;
		memset(filename,'\0',10);
		sprintf(filename,"%d",i);
		// Blocco dati di confronto.
		int n_byte=i*400;
		char* buf=(char*)malloc(n_byte*strlen(str)*sizeof(char)+1);
		if(check_eq(buf,NULL,"malloc buf retrieve")) {
			free(filename);
			return 0;
		}
		memset(buf,'\0',n_byte*strlen(str)*sizeof(char)+1);
		for(int j=0; j<n_byte; j++)
			strncat(buf,str,strlen(str));
		// Blocco dati da estrarre.
		char* block;
		s->op_eff++;
		block=(void*)os_retrieve(filename);
		if(block==NULL)
			s->op_fall++;
		else {
			s->op_succ++;
			if(strncmp(buf,block,n_byte)==0)
				s->op_conf++;
			else
				s->op_confall++;
		}
		free(filename);
		free(buf);
	}
	return 1;
}

/*
** Questa funzione elimina i file "i" all'interno della cartella del client.
** @param: -s: è la struttura dati utilizzata per l'aggiornamento delle stat.
** @return: la funzione ritorna 1 se è stata completata con successo, 0 se con errore.
*/
int delete(s_t* s) {
	for(int i=1; i<=N; i++) {
		// Nome del file=i.
		char* filename=(char*)malloc(10*sizeof(char));
		if(check_eq(filename,NULL,"malloc filename delete"))
			return 0;
		memset(filename,'\0',10);
		sprintf(filename,"%d",i);
		s->op_eff++;
		if(!os_delete(filename))
			s->op_fall++;
		else
			s->op_succ++;
		free(filename);
	}
	return 1;
}

/* 
** E' il client di test che opera con il programma server_objstr.c.
** Il client prende in input il nome del client per registrarsi e un numero
** da 1 a 3 per indicare la batteria di test da eseguire.
*/

int main(int argc, char** argv) {
	if(argc!=3) {
		printf("Use: %s name test_number",argv[0]);
		return(EXIT_FAILURE);
	}

	s_t stat;
	stat.op_eff=0;
	stat.op_succ=0;
	stat.op_fall=0;
	stat.op_conf=0;
	stat.op_confall=0;

	stat.op_eff++;
	if(!os_connect(argv[1])) {
		stat.op_fall++;
		goto end;
	}
	else
		stat.op_succ++;

	if(strcmp(argv[2],"1")==0)
		store(&stat);
	else {
		 if(strcmp(argv[2],"2")==0)
			retrieve(&stat);
		else {
			if(strcmp(argv[2],"3")==0)
				delete(&stat);
			else
				printf("Nessuna operazione disponibile\n");
		}
	}

	stat.op_eff++;
	if(!os_disconnect())
		stat.op_fall++;
	else
		stat.op_succ++;
	end:
	printf("Utente %s Test %s %d %d %d\n",argv[1],argv[2],stat.op_eff,stat.op_succ,stat.op_fall);
	/*printf("Operazioni effettuate: %d\n",stat.op_eff);
	printf("Operazioni completate con successo: %d\n",stat.op_succ);
	printf("Operazioni fallite: %d\n",stat.op_fall);
	printf("Operazioni di confronto con successo: %d\n",stat.op_conf);
	printf("Operazioni di confronto fallite: %d\n",stat.op_confall);*/
	return 0;
}