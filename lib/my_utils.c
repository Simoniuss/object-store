#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <my_utils.h>

/* 
** Implementazione dell'interfaccia macro.h.
*/


int check_eq(void* var, void* val, char* str) {
	if(var==val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_neq(void* var, void* val, char* str) {
	if(var!=val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_gt(void* var, void* val, char* str) {
	if(var>val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_lt(void* var, void* val, char* str) {
	if(var<val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_eq_err(int var, int val, char* str) {
	if(var==val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		errno=var;
		perror(str);
		return 1;
	}
	return 0;
}

int check_neq_err(int var, int val, char* str) {
	if(var!=val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		errno=var;
		perror(str);
		return 1;
	}
	return 0;
}

int check_gt_err(int var, int val, char* str) {
	if(var>val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		errno=var;
		perror(str);
		return 1;
	}
	return 0;
}

int check_lt_err(int var, int val, char* str) {
	if(var<val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		errno=var;
		perror(str);
		return 1;
	}
	return 0;
}

int check_eqint(int var, int val, char* str) {
	if(var==val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_neqint(int var, int val, char* str) {
	if(var!=val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_gtint(int var, int val, char* str) {
	if(var>val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}

int check_ltint(int var, int val, char* str) {
	if(var<val) {
		fprintf(stderr,"File:%s Line:%d ",__FILE__,__LINE__);
		perror(str);
		return 1;
	}
	return 0;
}