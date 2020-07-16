#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// incluir clase

cache_t *create_cache(size_t size, size_t asoc, size_t sets) {
}

int tracefile_parser() {
}

int main(int argc, char const *argv[]) {
	// ./cachesim tracefile.xex C E S [ -v n m ]
	// el archivo de traza a simular
	// el tamaño de la caché C, en bytes
	// la asociatividad de la caché, E
	// el número de sets de la caché, S

	/* Condiciones de error
	si el número de argumentos no es 4 o 7;
	si el archivo de trazas especificado no existe;
	si alguno de los parámetros C, E o S no son potencia de dos;
	si los argumentos n y m del modo verboso no son números enteros que cumplan 0 ≤ n ≤ m
	*/
	cache();
	return 0;
}
