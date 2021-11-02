#if !defined(OBJ_STR_H)
#define OBJ_STR_H

/*
** Insieme di funzioni necessarie per la comunicazione da parte di
** un client verso un server object store.
*/

/*
** Inizia la connessione verso l'object store, registrando il cliente 
** con il nome dato.
** @param: -name: il nome del client che vuole registrarsi.
** @return: restituisce 1 se la connessione è andata a buon fine, altrimenti 0.
*/
int os_connect(char *name);

/*
** Richiede all'object store la memorizzazione di un blocco dati.
** @param: -name: il nome del dato da memorizzare.
**		   -block: il blocco dati da memorizzare.
**		   -len: la lunghezza del blocco dati.
** @return: restituisce 1 se la memorizzazione è andata a buon fine, 0 altrimenti.
*/
int os_store(char *name, void *block, size_t len);

/*
** Restituisce un blocco dati memorizzato all'interno dell'object store.
** @param: -name: il nome del blocco dati da recuperare.
** @return: restituisce un puntatore al blocco dati se il recupero ha avuto successo, NULL altrimenti.
*/
void *os_retrieve(char *name);

/*
** Cancella un blocco dati precedentemente memorizzato nell'object store.
** @param: -name: il nome del blocco dati da eliminare.
** @return: restituisce 1 se il blocco dati è stato eliminato con successo, 0 altrimenti.
*/
int os_delete(char *name);

/*
** Chiude la connessione con l'object store.
** @return: restituisce 1 se la connessione è stata chiusa correttamente, 0 altrimenti.
*/
int os_disconnect();


#endif