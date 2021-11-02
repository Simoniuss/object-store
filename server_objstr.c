#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <my_utils.h>
#include <server_op.h>
//#include <list.h>

#define SOCKNAME "./objstore.sock"		//path del socket.
#define MAXPATH 104						//lunghezza massima del path del socket.
#define MAXSOCK 256						//lunghezza massima letta dal socket.
#define DATAPATH "./data"				//path della cartella data.

// Lock per accedere alla cartella DATAPATH.
static pthread_mutex_t mtx_data=PTHREAD_MUTEX_INITIALIZER;
// Lock per accedere alla struttura delle statistiche.
static pthread_mutex_t mtx_stat=PTHREAD_MUTEX_INITIALIZER;
// Variabile che gestisce la terminazione del server.
volatile sig_atomic_t termina=0;
// Variabile che gestisce la richiesta di stampa delle statistiche.
volatile sig_atomic_t statsig=0; 
// Struttura delle statistiche.
st_t stats;
// Hash table dei client registrati.
//list_t* list;


void cleanup() {
	// Elimino il socket.
	unlink(SOCKNAME);
}

void gestore_termina(int sig) {
	termina=1;
}

void gestore_stat(int sig) {
	statsig=1;
}

/*
** Questa funzione inizializza la gestione di tutti i segnali che devono
** essere gestiti dal server SIGINT,SIGQUIT,SIGTERM per la terminazione,
** SIGUSR1 per la stampa delle statistiche e SIGPIPE da ignorare.
** @return: ritorna 1 se si sono verificati errori nell'inizializzazione,
**			stampando un messaggio dell'errore, altrimenti ritorna 0.
*/
int gestione_segnali() {
	sigset_t set;
	struct sigaction sa;
	int err=0;
	if(check_eqint(sigemptyset(&set),-1,"sigemptyset"))
		goto error;
	if(check_eqint(sigaddset(&set,SIGINT),-1,"sigaddINT"))
		goto error;
	if(check_eqint(sigaddset(&set,SIGQUIT),-1,"sigaddQUIT"))
		goto error;
	if(check_eqint(sigaddset(&set,SIGTERM),-1,"sigaddTERM"))
		goto error;
	if(check_eqint(sigaddset(&set,SIGUSR1),-1,"sigaddUSR1"))
		goto error;
	if(check_eqint(sigaddset(&set,SIGPIPE),-1,"sigaddPIPE"))
		goto error;
	if(check_neq_err(pthread_sigmask(SIG_SETMASK,&set,NULL),0,"pthread_sigmask"))
		goto error;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler=gestore_termina;
	if(check_eqint(sigaction(SIGINT,&sa,NULL),-1,"sigactionINT"))
		goto error;
	if(check_eqint(sigaction(SIGQUIT,&sa,NULL),-1,"sigactionQUIT"))
		goto error;
	if(check_eqint(sigaction(SIGTERM,&sa,NULL),-1,"sigactionTERM"))
		goto error;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler=gestore_stat;
	if(check_eqint(sigaction(SIGUSR1,&sa,NULL),-1,"sigactionUSR1"))
		goto error;
	memset(&sa,0,sizeof(sa));
	sa.sa_handler=SIG_IGN;
	if(check_eqint(sigaction(SIGPIPE,&sa,NULL),-1,"sigactionPIPE"))
		goto error;
	if(check_eqint(sigemptyset(&set),-1,"sigempty"))
		goto error;
	if(check_neq_err(pthread_sigmask(SIG_SETMASK,&set,NULL),0,"pthread_sigmask"))
		goto error;
	return err;
	error:
	err=1;
	return err;
}

/*
** Questa funzione crea la directory passata come argomento, se non esiste.
** @param: -dir_path: il path della directory da creare.
** @return: ritorna 1 se la directory non è stata creata, 0 altrimenti.
*/
int init_dir(char* dir_path) {
	int err=0;
	DIR* dir;
	if((dir=opendir(dir_path))==NULL) {
		if(errno==ENOENT) {
			if(check_eqint(mkdir(dir_path,0700),-1,"mkdir data"))
				err=1;
		}
		else {
			perror("opendir data");
			err=1;
		}
	}
	else
		closedir(dir);
	return err;
}

/*
** Questa funzione compara due elementi nella lista.
*/
/*int cmp(void* a, void* b) {
	cl_t* a1=(cl_t*)a;
	cl_t* b1=(cl_t*)b;
	return strcmp(a1->name,b1->name);
}*/

/* 
** Questa funzione distrugge un oggetto di tipo cl_t all'interno della lista.
*/
/*void destr(void* obj) {
	cl_t o=*(cl_t*)obj;
	free(o.name);
	free(o.path);
	pthread_mutex_destroy(&o.mtx);
	free(obj);
}*/

/* 
** Questa funzione inizializza la struttura delle statistiche e ritorna 0
** se è stata inizializzata con successo.
*/
int init_stat(st_t* s) {
	s=(st_t*)malloc(sizeof(st_t));
	if(s==NULL)
		return 1;
	s->conn=0;
	s->n_obj=0;
	s->size=0;
	return 0;
}

/*
** Questa funzione inizializza il listener per attendere nuove connessioni
** in entrata.
** @param: -fd_sock: è il puntatore al fd del listener che verrà inizializzato nella funzione.
** @return: ritorna 1 se l'inizializzazione del socket è fallita, con il relativo
**			messaggio d'errore, 0 se l'inizializzazione è andata a buon fine.
*/
int init_socket(int* fd_sock) {
	int err=0;
	struct sockaddr_un sa;
	strncpy(sa.sun_path,SOCKNAME,MAXPATH);
	sa.sun_family=AF_UNIX;
	if(check_eqint((*fd_sock=socket(AF_UNIX,SOCK_STREAM,0)),-1,"socket server"))
		goto error;
	if(check_eqint(bind(*fd_sock,(struct sockaddr*)&sa,sizeof(sa)),-1,"bind server"))
		goto error;
	if(check_eqint(listen(*fd_sock,SOMAXCONN),-1,"listen server"))
		goto error;
	return err;
	error:
	err=1;
	return err;
}

void* objstr(void* arg) {
	long conn=(long)arg;
	// c conterrà il lock del client e il suo nome.
	cl_t* c=(cl_t*)malloc(sizeof(cl_t));
	check_eq(c,NULL,"malloc c");
	c->name=(char*)malloc(256*sizeof(char)+1);
	check_eq(c->name,NULL,"malloc c->name");
	memset(c->name,'\0',256+1);
	c->path=(char*)malloc(4096*sizeof(char)+1);
	check_eq(c->path,NULL,"malloc c->path");
	memset(c->path,'\0',4096+1);
	char* rec_msg=(char*)malloc(MAXSOCK*sizeof(char)+1);
	if(check_eq(rec_msg,NULL,"malloc rec_msg"))
		goto err;
	memset(rec_msg,'\0',MAXSOCK+1);
	while(1) {
		int n=0;
		if(check_eqint((n=read(conn,rec_msg,MAXSOCK)),-1,"read operation"))
			goto err;
		if (n==0)
			break;
		char* tok=(char*)malloc(MAXSOCK*sizeof(char));
		if(check_eq(tok,NULL,"malloc token"))
			goto err;
		memset(tok,'\0',MAXSOCK);
		char* tmp;
		// Leggo l'operazione.
		tok=strtok_r(rec_msg," ",&tmp);
		// variabile per controllare se le singole operazioni sono andate a buon fine.
		int check=-1;
		if(strcmp(tok,"REGISTER")==0) {
			// Leggo il nome del client.
			tok=strtok_r(NULL," ",&tmp);
			check_neq_err(pthread_mutex_lock(&mtx_data),0,"lock mtx_data");
			check=s_connect(conn,DATAPATH,/*list,*/tok,c);
			check_neq_err(pthread_mutex_unlock(&mtx_data),0,"unlock mtx_data");
			if(check) {
				check_neq_err(pthread_mutex_lock(&mtx_stat),0,"lock stats register");
				stats.conn++;
				check_neq_err(pthread_mutex_unlock(&mtx_stat),0,"unlock stats register");
			}
		}
		else if(strcmp(tok,"STORE")==0) {
			int m=n-6;
			char* name=(char*)malloc(MAXSOCK*sizeof(char));
			if(check_eq(name,NULL,"malloc name file store"))
				goto err;
			memset(name,'\0',MAXSOCK);
			// Leggo il nome del file da archiviare.
			name=strtok_r(NULL," ",&tmp);
			m=m-strlen(name)-1;
			int len=0;
			// Leggo la lunghezza del file.
			tok=strtok_r(NULL," ",&tmp);
			len=strtol(tok,NULL,10);
			strtok_r(NULL," ",&tmp);
			m=m-strlen(tok)-1-2;
			char* block=(char*)malloc(len*sizeof(char)+1);
			if(check_eq(block,NULL,"malloc block store"))
				goto err;
			memset(block,'\0',len*sizeof(char)+1);
			// E' già arrivata una parte del blocco dati.
			if(m>0) {
				block=strtok_r(NULL," ",&tmp);
				m=strlen(block);
			}
			// Ho letto solo l'header dell'operazione.
			else
				m=0;
			// Se non ho letto tutti i dati con la prima read.
			while(m<len) {
				if(m==0) {
					if(check_eqint((n=read(conn,block,len)),-1,"read block 2 store"))
						goto err;
					m=n;
				}
				else {
					if(check_eqint((n=read(conn,block+m,len-m)),-1,"read block 2 store"))
						goto err;
					m=m+n;
				}
			}
			check_neq_err(pthread_mutex_lock(&mtx_data),0,"lock mtx_data store");
			check=s_store(conn,name,block,len,c->path);
			check_neq_err(pthread_mutex_unlock(&mtx_data),0,"unlock mtx_data store");
			if(check) {
				check_neq_err(pthread_mutex_lock(&mtx_stat),0,"lock stats store");
				stats.n_obj++;
				stats.size+=len;
				check_neq_err(pthread_mutex_unlock(&mtx_stat),0,"unlock stats store");
			}
		}
		else if(strcmp(tok,"RETRIEVE")==0) {
			// Leggo il nome del file da recuperare.
			tok=strtok_r(NULL," ",&tmp);
			check_neq_err(pthread_mutex_lock(&mtx_data),0,"lock mtx_data retrieve");
			s_retrieve(conn,tok,c->path);
			check_neq_err(pthread_mutex_unlock(&mtx_data),0,"unlock mtx_data retrieve");
		}
		else if(strcmp(tok,"DELETE")==0) {
			// Leggo il nome del file da eliminare.
			tok=strtok_r(NULL," ",&tmp);
			check_neq_err(pthread_mutex_lock(&mtx_data),0,"lock mtx_data delete");
			check=s_delete(conn,tok,c->path);
			check_neq_err(pthread_mutex_unlock(&mtx_data),0,"unlock mtx_data delete");
			if(check>=0) {
				check_neq_err(pthread_mutex_lock(&mtx_stat),0,"lock stats delete");
				stats.n_obj--;
				stats.size-=check;
				check_neq_err(pthread_mutex_unlock(&mtx_stat),0,"unlock stats delete");
			}
		}
		else if(strcmp(tok,"LEAVE")==0) {
			s_leave(conn);
			check_neq_err(pthread_mutex_lock(&mtx_stat),0,"lock stats leave");
			stats.conn--;
			check_neq_err(pthread_mutex_unlock(&mtx_stat),0,"unlock stats leave");
			break;
		}
		// Resetto il buffer che legge l'operazione.
		memset(rec_msg,'\0',MAXSOCK);
		memset(tok,'\0',MAXSOCK);
		check=-1;
	}
	return NULL;
	err:
	ko(conn,"objstr: ",strerror(errno));
	return NULL;
}

/*
** Questa funzione genera un worker thread per ogni client connesso e servirà
** esclusivamente le richieste del client. Il thread viene creato in modalità
** detached così una volta che ha completato di servire il client terminerà la
** sua esecuzione senza un'attesa da parte del server. Se la creazione del thread
** fallisce la connessione viene chiusa inviando un KO message.
** @param: -conn: il descrittore del socket dedicato alla connessione.
*/
void worker_spawn(long conn) {
	pthread_t th;
	pthread_attr_t th_attr;
	int e=0;

	if((e=pthread_attr_init(&th_attr))!=0)
		goto err;
	if((e=pthread_attr_setdetachstate(&th_attr,PTHREAD_CREATE_DETACHED))!=0){
		pthread_attr_destroy(&th_attr);
		goto err;
	}
	if((e=pthread_create(&th,&th_attr,&objstr,(void*)conn))!=0) {
		pthread_attr_destroy(&th_attr);
		goto err;
	}
	return;
	err:
	errno=e;
	ko(conn,"thred_spawn",strerror(errno));
}


int main() {
	cleanup();
	atexit(cleanup);
	if(gestione_segnali())
		return(EXIT_FAILURE);
	if(init_dir(DATAPATH))
		return(EXIT_FAILURE);
	if(init_stat(&stats))
		return(EXIT_FAILURE);
	/*if((list=list_new())==NULL)
		return(EXIT_FAILURE);
	list->match=cmp;*/
	int fd_skt;
	if(init_socket(&fd_skt)) {
		perror("init_socket");
		return(EXIT_FAILURE);
	}
	int fd_c;

	//TEST
	//worker_spawn(0);
	//TEST

	while(!termina) {
		if(statsig) {
			print_stat(stats);
			statsig=0;
		}
		if((fd_c=accept(fd_skt,NULL,0))==-1) {
			if(errno==EINTR) {
				if(termina)
					break;
			}
			else
				perror("accept");
		}
		else {
			worker_spawn(fd_c);
		}
	}
	return 0;
}