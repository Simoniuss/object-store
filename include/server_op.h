#if !defined(SERVER_OP_H)
#define SERVER_OP_H

//#include <list.h>
#include <pthread.h>

/*
** Funzioni necessarie all'esecuzione delle richieste ricevute all'interno
** dell'object store.
*/

/*
** Questa è la struttura di gestione dei client, (dove ogni client ha un lock
** per la proprio cartella.)
** @param: -name: è il nome del client connesso.
**		   -path: è il path della sa directory.
** 		   (-mtx: è il lock dedicato al client.)
*/
typedef struct clients_t {
	char* name;
	char* path;
	//pthread_mutex_t mtx;
}cl_t;

/*
** Questa struttura contiene le statistiche di cui il server dovrà tenere traccia.
** @param: -conn: numero di connessioni attive.
**		   -n_obj: numero di oggetti salvati nell'object store.
**		   -size: dimensione totale dell'object store.
*/
typedef struct stats_t {
	int conn;
	int n_obj;
	int size;
}st_t;

/*
** Questa funzione invia il messaggio di OK nel socket utilizzato per la connessione.
** @param: -conn: è il descrittore del socket.
*/
void ok(long conn);

/*
** Questa funzione scrive il messaggio di KO sul socket utilizzato per la conessione.
** @param: -conn: è il descrittore del socket.
**		   -str: è una stringa per comprendere meglio l'errore.
**		   -strerr: è il tipo di errore verificato. (da utilizzare con strerror(errno)).
*/
void ko(long conn, char* str, char*strerr);

/*
** Questa funzione invia il messaggio DATA sul socket di connessione.
** @param: -conn: è il descrittore del socket.
**		   -data: è il blocco dati da inviare.
**		   -len: è la lunghezza del blocco dati.
*/
void data(long conn, void* data, size_t len);

/*
** Crea una nuova cartella all'interno della cartella "data", con nome "name",
** solo se questo nome non è già esistente, altrimenti vi si connette soltanto.
** Scrive sul canale di comunicazione se l'operazione è andata a buon fine oppure no.
** (Inoltre riempie la struttura cl_t relativa al cliente corrente, in modo da
** accedervi successivamente senza dover rileggere la lista dei client.)
** @param: -name: il nome dell'utente da registrare.
**		   -conn: è il descrittore del canale di comunicazione.
**		   -data_path: è il path della directory nella quale cercare o creare "name".
**		   (-l: è la lista che contiene tutti i clienti.)
**		   -c: è il puntatore alla struttura dati del cliente attuale.
** @return: 1 se è andata a buon fine, 0 altrimenti.
*/
int s_connect(long conn, char* data_path, /*list_t* l, */char* name, cl_t* c);

/*
** Salva un nuovo file all'interno della cartella dell'utente corrente,
** con nome "name", solo se questo nome non è già esistente. Il file è
** riempito con blocco dati "block" di grandezza "len". Scrive sul canale 
** di comunicazione se l'operazione è andata a buon fine oppure no.
** @param: -name: il nome del nuovo file.
**		   -conn: è il descrittore del canale di comunicazione.
**		   -block: i dati da inserire nel file.
**		   -len: la lunghezza dei dati.
**		   -cl_path: il path dove andare a creare il nuovo file.
** @return: 1 se è andata a buon fine, 0 altrimenti.
*/
int s_store(long conn, char* name, void* block, size_t len, char* cl_path);

/*
** Recupera il blocco dati con nome "name". Invia sul canale di comunicazione
** il blocco se è stato trovato oppure un messaggio d'errore.
** @param: -name: il nome del file da cercare.
**		   -conn: è il descrittore del canale di comunicazione.
**		   -cl_path: il path dove cercare il file.
** @return: 1 se è andata a buon fine, 0 altrimenti.
*/
int s_retrieve(long conn, char* name, char* cl_path);

/*
** Elimina il file con nome "name" se esiste. Scrive sul canale di comunicazione
** se l'operazione è andata a buon fine oppure no.
** @param: -name: il nome del file da eliminare.
**		   -conn: è il descrittore del canale di comunicazione.
**		   -cl_path: il path dove si trova il file da eliminare.
** @return: ritorna la size del file eliminato.
*/
int s_delete(long conn, char* name, char* cl_path);

/*
** Chiude la connessione con il client. Scrive sul canale di comunicazione
** l'andata a buon fine.
** @param: -conn: è il descrittore del canale di comunicazione.
*/
void s_leave(long conn);

/*
** Questa funzione stampa il contenuto della struttura s che contiene le statistiche.
** @param: -s: è la struttura che contiene le statistiche.
*/
void print_stat(st_t s);


#endif