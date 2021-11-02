#if !defined(MY_UTILS_H)
#define MY_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

/*
** Una raccolta di funzioni utili da utilizzare per controllare eventuali
** errori in uscita dopo chiamate di funzioni.
*/

/*
** La funzione verifica che la variabile abbia il valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_eq(void* var, void* val, char* str);

/*
** La funzione verifica che la variabile abbia valore diverso dal
** valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/

int check_neq(void* var, void* val, char* str);

/*
** La funzione verifica che la variabile sia maggiore del valore assgenato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_gt(void* var, void* val, char* str);

/*
** La funzione verifica che la variabile sia minore del valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_lt(void* var, void* val, char* str);

/*
** La funzione verifica che la variabile abbia il valore assegnato e 
** setta errno con il valore della variabile.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_eq_err(int var, int val, char* str);

/*
** La funzione verifica che la variabile abbia valore diverso dal
** valore assegnato e setta errno con il valore della variabile.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/

int check_neq_err(int var, int val, char* str);

/*
** La funzione verifica che la variabile sia maggiore del valore assegnato
** e setta errno con il valore della variabile.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_gt_err(int var, int val, char* str);

/*
** La funzione verifica che la variabile sia minore del valore assegnato e
** setta errno con il valore della variabile.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_lt_err(int var, int val, char* str);

/*
** La funzione verifica che la variabile abbia il valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_eqint(int var, int val, char* str);

/*
** La funzione verifica che la variabile abbia valore diverso dal
** valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/

int check_neqint(int var, int val, char* str);

/*
** La funzione verifica che la variabile sia maggiore del valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_gtint(int var, int val, char* str);

/*
** La funzione verifica che la variabile sia minore del valore assegnato.
** @param: la variabile da confrontare, il valore di confronto e la
**			stringa di descrizione dell'errore.
** @return: restituisce 1 se il confronto restituisce true, altrimenti 0.
*/
int check_ltint(int var, int val, char* str);



#endif