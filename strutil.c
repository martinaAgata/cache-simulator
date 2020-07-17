#define _POSIX_C_SOURCE 200809L
#include "strutil.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void imprimir_strv(char** strv);

void free_strv(char* strv[]) {
	if (strv == NULL) return;
	for (int i = 0; strv[i]; i++) free(strv[i]);
	free(strv);
}

char** split(const char* str, char sep) {
	if (str == NULL) return NULL;
	size_t contador = 2;
	int i = 0;
	while (str[i]) if(str[i++] == sep) contador++;
	char** strv = malloc(contador*sizeof(char*));
	if(!strv) return NULL;

	size_t len = 0;
	int j = 0, k = 0;
	if (!str[j] && !len) {
		strv[k] = malloc(sizeof(char));
		if(!strv[k]) return NULL;
		strv[k++][0] = '\0';
	}
	while (str[j]) {
		if (str[j] == sep || !str[j+1]) {
			if (!str[j+1] && str[j] != sep) {
				len++;
				j++;
			}
			strv[k] = malloc((len + 1)*sizeof(char));
			if (!strv[k]) return NULL;
			memcpy(strv[k], &str[j-len], len);
			strv[k++][len] = '\0';
			len = 0;
			if(str[j] != '\0' && str[j] == sep && !str[j+1]) {
				strv[k] = malloc(sizeof(char));
				if(!strv[k]) return NULL;
				strv[k++][0] = '\0';
			}
		}
		else len++;
		if(str[j]) j++;
	}
	strv[k] = NULL;
	return strv;
}

char* join(char** strv, char sep) {
	if (strv == NULL) return NULL;
	if (strv[0] == NULL) return strdup("");
	size_t totales = 0;
	for (int i = 0; strv[i]; i++) totales += (strlen(strv[i]) + 1);
	char* cadena = malloc(totales * sizeof(char));
	if (!cadena) return NULL;
	size_t longitud_actual = 0;
	for (int i = 0; strv[i] != NULL; i++) {
		strcpy(cadena + longitud_actual, strv[i]);
		longitud_actual += strlen(strv[i]);
		if (strv[i+1]) cadena[longitud_actual++] = sep;
	}
	return cadena;
}

void imprimir_strv(char** strv) {
	if (strv == NULL) return;
	int j = 0;
	while (strv[j]) {
		printf("#%s# ", strv[j]);
		j++;
	}
	printf("\n");
}
