# object-store
 Implementazione del progetto per il corso di "Laboratorio di Sistemi Operativi" 2018-19 presso l'Università di Pisa. Il progetto prevede l'implementazione di un sistema client-server per memorizzare e restituire blocchi di dati, simile a uno storage locale.


 ##  Table of Contents
 * [Introduction](#introduction)
 * [Installation](#installation)
 * [Usage](*usage)
 * [Test](*test)


 ## Introduction
 L'object store è un eseguibile il cui scopo è quello di ricevere dai client delle richieste di memorizzare, recuperare, cancellare blocchi di dati dotati di nome, detti “oggetti”. L'object store gestisce uno spazio di memorizzazione separato per ogni cliente, con nomi univoci, e i nomi degli oggetti sono garantiti essere univoci all'interno dello spazio di memorizzazione di un cliente. Tutti i nomi rispettano il formato dei nomi di file POSIX.  L'object store è un server che attende il collegamento di un client su una socket locale. Per collegarsi all'object store, un client invia al server un messaggio di registrazione e in risposta l'object store crea un thread destinato a servire le richieste di quel particolare cliente. 
 I dettagli del protocollo sono dati nel seguito:

 * **os_connect**(char* name): inizia la connessione all'object store, registrando il cliente con _name_.
 * **os_store**(char *name, void *block, size_t len): richiede all'object store la memorizzazione dell'oggetto puntato da _block_, per una lunghezza _len_, con il nome _name_.
 * **os_retrieve**(char *name): recupera dall'object store l'oggetto precedentemente memorizzatato sotto il nome _name_.
* **os_delete**(char *name): cancella l'oggetto di nome _name_ precedentemente memorizzato.
* **os_disconnect**(): chiude la connessione all'object store.


 ## Installation
```
$ git clone https://github.com/Simoniuss/object-store
$ cd object-store
$ make all
```


 ## Usage
```
$ ./server_objstr
```


## Test
```
$ make test
$ ./testsum.sh
```