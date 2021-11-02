#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <my_utils.h>
#include <server_op.h>
//#include <list.h>

#define MAXPATH 4096		//lunghezza massima path.
#define MAXNAME 256			//lunghezza massima nome cartella.


/*
** Implementazione dell'interfaccia server_op.h.
*/

void ok(long conn) {
	char* s=(char*)malloc(6*sizeof(char)+1);
	memset(s,'\0',6+1);
	s="OK \n";
	check_neqint(write(conn,s,4),4,"write ok");
}

void ko(long conn, char* str, char* strerr) {
	int n=0;
	n=strlen(str)+strlen(strerr);
	char* s=(char*)malloc((n+6)*sizeof(char)+1);
	memset(s,'\0',n+6+1);
	strncpy(s,"KO ",3);
	strncat(s,str,strlen(str));
	strncat(s,strerr,strlen(strerr));
	strncat(s,"\n",1);
	check_neqint(write(conn,s,strlen(s)),strlen(s),"write ko");
}

void data(long conn, void* data, size_t len) {
	char* s=(char*)malloc((len+15)*sizeof(char)+1);
	memset(s,'\0',(len+15)*sizeof(char)+1);
	strncpy(s,"DATA ",5);
	char l[10];
	sprintf(l,"%zu",len);
	strncat(s,l,strlen(l));
	strncat(s," \n ",3);
	strncat(s,data,len);
	check_neqint(write(conn,s,strlen(s)),strlen(s),"write data");
}

int s_connect(long conn, char* data_path, /*list_t* l,*/ char* name, cl_t* c) {
	if(strcmp(name,".")==0 || strcmp(name,"..")==0) {
		ko(conn,"connect: ",strerror(EINVAL));
		return 0;
	}

	DIR* data=NULL;
	if(check_eq(data=opendir(data_path),NULL,"opendir data"))
		goto err;
	errno=0;
	struct dirent* cl_dir=readdir(data);
	while(cl_dir!=NULL && strcmp(cl_dir->d_name,name)!=0)
		cl_dir=readdir(data);
	// Entro in "data" per creare o aprire la cartella "name".
	if(check_eqint(chdir(data_path),-1,"chdir"))
		goto err;
	// In questo case "name" non esiste, quindi devo crearlo, oppure si è verificato un errore.
	//int make=0;
	if(cl_dir==NULL) {
		if(errno==0) {
			if(check_eqint(mkdir(name,0700),-1,"mkdir name"))
				goto err;
			//make=1;
		}
		else
			goto err;
	}
	if(check_eqint(closedir(data),-1,"closedir data"))
		goto err;

	// (La cartella è stata create quindi devo inserirla nella lista.)
	if(1) {
		strncpy(c->name,name,strlen(name));
		getcwd(c->path,MAXPATH);
		strncat(c->path,"/",1);
		strncat(c->path,name,strlen(name));
		strncat(c->path,"/",1);
		//if(check_neq_err(pthread_mutex_init(&c->mtx,NULL),0,"init c->mtx"))
		//	goto err;
		//list_node_t* node=list_node_new(c);
		//list_lpush(l,node);
	}
	// La cartella non è stata creata quindi la cerco nella lista.
	/*else {
		cl_t* obj=(cl_t*)malloc(sizeof(cl_t));
		obj->name=(char*)malloc(MAXNAME*sizeof(char));
		obj->path=(char*)malloc(MAXPATH*sizeof(char));
		list_iterator_t* it=list_iterator_new(l,LIST_HEAD);
		list_node_t* node;
		int trovato=0;
		while(!trovato) {
			printf("in while\n");
			node=list_iterator_next(it);
			printf("next node\n");
			obj=(cl_t*)(node->val);
			printf("name:%s\n",obj->name);
			if(strcmp(obj->name,name)==0) {
				printf("confronto\n");
				trovato=1;
				list_iterator_destroy(it);
			}
		}
		if(check_neq_err(pthread_mutex_init(&c->mtx,NULL),0,"init c->mtx"))
			goto err;
		c->mtx=obj->mtx;
		strncpy(c->name,obj->name,strlen(obj->name));
		strncpy(c->path,obj->path,strlen(obj->path));
	}*/
	if(check_eqint(chdir("../"),-1,"chdir return from data"))
		goto err;
	ok(conn);
	return 1;
	err:
	if(data)
		closedir(data);
	//if(pthread_mutex_trylock(&c->mtx)==0)
	//	pthread_mutex_destroy(&c->mtx);
	ko(conn,"connect: ",strerror(errno));
		return 0;
}

int s_store(long conn, char* name, void* block, size_t len, char* cl_path) {
	int new=0;
	char* path=(char*)malloc(MAXPATH*sizeof(char)+1);
	if(check_eq(path,NULL,"malloc path client"))
		goto err;
	memset(path,'\0',MAXPATH+1);
	strncpy(path,cl_path,strlen(cl_path));
	strncat(path,name,strlen(name));
	// Creo un nuovo file se non esiste, altrimenti restituisco ko.
	if((new=open(path,O_WRONLY|O_CREAT|O_EXCL,0700))==-1)
		goto err;
	if(check_neqint(write(new,block,len),len,"write block in file"))
		goto err;
	close(new);
	ok(conn);
	return 1;
	err:
	if(new!=0)
		close(new);
	ko(conn,"store: ",strerror(errno));
	return 0;
}

int s_retrieve(long conn, char* name, char* cl_path) {
	DIR* dir;
	if(check_eq(dir=opendir(cl_path),NULL,"opendir client"))
		goto err;
	if(check_eqint(chdir(cl_path),-1,"chdir client"))
		goto err;
	struct dirent* d=readdir(dir);
	while(d!=NULL) {
		if(strcmp(d->d_name,".")!=0 && strcmp(d->d_name,"..")!=0) {
			// Il file è stato trovato, lo devo restituire.
			if(strcmp(d->d_name,name)==0) {
				char* d_path=(char*)malloc(MAXPATH*sizeof(char)+1);
				if(check_eq(d_path,NULL,"malloc d_path"))
					goto err;
				memset(d_path,'\0',MAXPATH+1);
				strncpy(d_path,cl_path,strlen(cl_path));
				strncat(d_path,d->d_name,strlen(d->d_name));
				int f=0;
				if(check_eqint(f=open(d_path, O_RDONLY),-1,"open file retrieve"))
					goto err;
				// Leggo la grandezza del file.
				struct stat s;
				if(check_eqint(fstat(f,&s),-1,"fstat file"))
					goto err;
				int size=s.st_size;
				void* buf=(void*)malloc(size+1);
				if (check_eq(buf,NULL,"malloc buf retrieve"))
					goto err;
				memset(buf,'\0',size+1);
				if(check_neqint(read(f,buf,size),size,"read file"))
					goto err;
				data(conn,buf,size);
				if(check_eqint(chdir("../../"),-1,"chdir return from client dir"))
					goto err;
				free(d_path);
				close(f);
				closedir(dir);
				return 1;
			}
		}
		d=readdir(dir);
	}
	// Il file non esiste.
	if(d==NULL) {
		ko(conn,"retrieve: ",strerror(ENOENT));
		closedir(dir);
		if(check_eqint(chdir("../../"),-1,"chdir return from client dir"))
			goto err;
		return 0;
	}
	err:
	if(dir)
		closedir(dir);
	check_eqint(chdir("../../"),-1,"chdir return from client dir");
	ko(conn,"retrieve: ",strerror(errno));
	return 0;
}

int s_delete(long conn, char* name, char* cl_path) {
	int size=0;
	DIR* dir;
	if(check_eq(dir=opendir(cl_path),NULL,"opendir client dir"))
		goto err;
	if(check_eqint(chdir(cl_path),-1,"chdir client dir"))
		goto err;
	struct dirent* d=readdir(dir);
	while(d!=NULL) {
		if(strcmp(d->d_name,".")!=0 && strcmp(d->d_name,"..")!=0) {
			// Il file è stato trovato, lo devo cancellare.
			if(strcmp(d->d_name,name)==0) {
				char* d_path=(char*)malloc(MAXPATH*sizeof(char)+1);
				if(check_eq(d_path,NULL,"malloc d_path"))
					goto err;
				memset(d_path,'\0',MAXPATH+1);
				strncpy(d_path,cl_path,strlen(cl_path));
				strncat(d_path,d->d_name,strlen(d->d_name));
				int f=0;
				if(check_eqint(f=open(d_path, O_RDONLY),-1,"open file delete"))
					goto err;
				// Leggo la grandezza del file.
				struct stat s;
				if(check_eqint(fstat(f,&s),-1,"fstat file delete"))
					goto err;
				size=s.st_size;
				close(f);
				remove(d_path);
				ok(conn);
				if(check_eqint(chdir("../../"),-1,"chdir return from client dir"))
					goto err;
				closedir(dir);
				free(d_path);
				return size;
			}
		}
		d=readdir(dir);
	}
	// Il file non esiste.
	if(d==NULL) {
		ko(conn,"delete: ",strerror(ENOENT));
		closedir(dir);
		if(check_eqint(chdir("../../"),-1,"chdir return from client dir"))
			goto err;
		return size;
	}
	err:
	if(dir)
		closedir(dir);
	check_eqint(chdir("../../"),-1,"chdir return from client dir");
	ko(conn,"retrieve: ",strerror(errno));
	return size;
}

void s_leave(long conn) {
	ok(conn);
	return;
}

void print_stat(st_t s) {
	printf("Client connessi: %d\n",s.conn);
	printf("Oggetti memorizzati: %d\n",s.n_obj);
	printf("Size dello store: %d\n",s.size);
}