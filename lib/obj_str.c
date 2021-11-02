#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <obj_str.h>
#include <my_utils.h>

// Lunghezza massima letta o scritta nel socket.
#define MAXSOCK 256
#define SOCKNAME "./objstore.sock"

// E' il descrittore di comunicazione.
static int fd=-1;

/*
** Implementazione dell'interfaccia obj_str.h.
*/

int os_connect(char *name) {
	struct sockaddr_un sa;
	strncpy(sa.sun_path,SOCKNAME,strlen(SOCKNAME)+1);
	sa.sun_family=AF_UNIX;
	if(check_eqint((fd=socket(AF_UNIX,SOCK_STREAM,0)),-1,"socket client"))
		goto err;
	int try=0;
	while((connect(fd,(struct sockaddr*)&sa,sizeof(sa)))==-1 && try<5) {
		if(errno==ENOENT) {
			try++;
			sleep(1);
		}
		else
			goto err;
	}
	char* send_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(send_msg,NULL,"malloc send_msg connect client"))
		goto err;
	memset(send_msg,'\0',MAXSOCK);
	strncpy(send_msg,"REGISTER ",9);
	strncat(send_msg,name,strlen(name));
	strncat(send_msg," \n",2);
	if(check_eqint(write(fd,send_msg,strlen(send_msg)),-1,"write connect client"))
		goto err;
	char* rec_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(rec_msg,NULL,"malloc rec_msg connect"))
		goto err;
	memset(rec_msg,'\0',MAXSOCK);
	int n=-1;
	if(check_eqint((n=read(fd,rec_msg,MAXSOCK)),-1,"read rec_msg connect client"))
		goto err;
	if(strcmp(rec_msg,"OK \n")==0) {
		free(send_msg);
		free(rec_msg);
		return 1;
	}
	free(send_msg);
	free(rec_msg);
	return 0;
	err:
	if(fd!=-1)
		close(fd);
	return 0;
}

int os_store(char *name, void *block, size_t len) {
	if (fd==-1)
		return 0;
	char* send_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(send_msg,NULL,"malloc send_msg store client"))
		goto err;
	memset(send_msg,'\0',MAXSOCK);
	strncpy(send_msg,"STORE ",6);
	strncat(send_msg,name,strlen(name));
	strncat(send_msg," ",1);
	char l[30];
	sprintf(l,"%zu",len);
	strncat(send_msg,l,strlen(l));
	strncat(send_msg," \n ",3);
	if(check_eqint(write(fd,send_msg,strlen(send_msg)),-1,"write send_msg store"))
		goto err;
	if(check_eqint(write(fd,block,len),-1,"write block store client"))
		goto err;
	char* rec_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(rec_msg,NULL,"malloc rec_msg store client"))
		goto err;
	memset(rec_msg,'\0',MAXSOCK);
	int n=-1;
	if(check_eqint((n=read(fd,rec_msg,MAXSOCK)),-1,"read rec_msg store client"))
		goto err;
	if(strcmp(rec_msg,"OK \n")==0) {
		free(send_msg);
		free(rec_msg);
		return 1;
	}
	free(send_msg);
	free(rec_msg);
	return 0;
	err:
	if(fd!=-1)
		close(fd);
	return 0;
}

void *os_retrieve(char *name) {
	if (fd==-1)
		return NULL;
	char* send_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(send_msg,NULL,"malloc send_msg retrieve client"))
		goto err;
	memset(send_msg,'\0',MAXSOCK);
	strncpy(send_msg,"RETRIEVE ",9);
	strncat(send_msg,name,strlen(name));
	strncat(send_msg," \n",2);
	if(check_eqint(write(fd,send_msg,strlen(send_msg)),-1,"write send_masg retrieve client"))
		goto err;
	char* rec_msg=(char*)malloc(MAXSOCK*sizeof(char)+1);
	if(check_eq(rec_msg,NULL,"malloc rec_msg retrieve client"))
		goto err;
	memset(rec_msg,'\0',MAXSOCK*sizeof(char)+1);
	int n=-1;
	if(check_eqint((n=read(fd,rec_msg,MAXSOCK)),-1,"read rec_msg retrieve client"))
		goto err;
	char* tok;
	char* tmp;
	// byte del primo blocco letto, può contenere o meno un pò di dati.
	int m=n;
	tok=strtok_r(rec_msg," ",&tmp);
	m=m-strlen(tok)-1;
	if(strcmp(tok,"DATA")==0) {
		// Leggo la grandezza del file.
		tok=strtok_r(NULL," ",&tmp);
		int l=strtol(tok,NULL,10);
		// tolgo da m la lunghezza della size, lo spazio, lo \n e l'altro spazio.
		m=m-strlen(tok)-1-2;
		char* block=(char*)malloc(l+1);
		if(check_eq(block,NULL,"malloc block retrieve client"))
			goto err;
		memset(block,'\0',l+1);
		//lettura a vuoto dello spazio tra \n e il blocco dati.
		strtok_r(NULL," ",&tmp);
		//Leggo una parte del blocco letto nella prima lettura.
		block=strtok_r(NULL," ",&tmp);
		while(m<l) {
			char* block2=(char*)malloc(l-m+1);
			if(check_eq(block2,NULL,"malloc block2 retrieve client"))
				goto err;
			memset(block2,'\0',l-m+1);
			if(check_eqint((n=read(fd,block2,l-m)),-1,"read block2 retrieve client"))
				goto err;
			memcpy(block+m,block2,n);
			m=m+n;
			free(block2);
		}
		free(send_msg);
		return (void*)block;
	}
	free(send_msg);
	return NULL;
	err:
	if(fd!=-1)
		close(fd);
	return NULL;
}

int os_delete(char *name) {
	if (fd==-1)
		return 0;
	char* send_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(send_msg,NULL,"malloc send_msg delete client"))
		goto err;
	memset(send_msg,'\0',MAXSOCK);
	strncpy(send_msg,"DELETE ",7);
	strncat(send_msg,name,strlen(name));
	strncat(send_msg," \n",2);
	if(check_eqint(write(fd,send_msg,strlen(send_msg)),-1,"write send_msg delete client"))
		goto err;
	char* rec_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(rec_msg,NULL,"malloc rec_msg delete client"))
		goto err;
	memset(rec_msg,'\0',MAXSOCK);
	int n=-1;
	if(check_eqint((n=read(fd,rec_msg,MAXSOCK)),-1,"read rec_msg delete client"))
		goto err;
	if(strcmp(rec_msg,"OK \n")==0) {
		free(send_msg);
		free(rec_msg);
		return 1;
	}
	free(send_msg);
	free(rec_msg);
	return 0;
	err:
	if(fd!=-1)
		close(fd);
	return 0;
}

int os_disconnect() {
	if (fd==-1)
		return 0;
	char* send_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(send_msg,NULL,"malloc send_msg disconnect client"))
		goto err;
	memset(send_msg,'\0',MAXSOCK);
	strncpy(send_msg,"LEAVE \n",6);
	if(check_eqint(write(fd,send_msg,6),-1,"write send_msg disconnect client"))
		goto err;
	char* rec_msg=(char*)malloc(MAXSOCK*sizeof(char));
	if(check_eq(rec_msg,NULL,"malloc rec_msg disconnect client"))
		goto err;
	memset(rec_msg,'\0',MAXSOCK);
	int n=-1;
	if(check_eqint((n=read(fd,rec_msg,MAXSOCK)),-1,"read rec_msg disconnect client"))
		goto err;
	if(strcmp(rec_msg,"OK \n")==0) {
		free(send_msg);
		free(rec_msg);
		return 1;
	}
	free(send_msg);
	free(rec_msg);
	return 0;
	err:
	if(fd!=-1)
		close(fd);
	return 0;
}