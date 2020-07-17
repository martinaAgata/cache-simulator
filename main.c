#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "strutil.h"

/********************* Estructuras provisorias *********************/

typedef struct {
	int c;
	int e;
	int s;
	int block_bytes;
} cache_t;

typedef struct {
	int valid;
	int dirty;
	int tag;
	int block;
} line_t;

typedef struct {
	line_t *lines;
	int size; // size = E
} set_t;

/*******************************************************************/


int cache_read(cache_t *cache, int memory_address, size_t bytes_amount, int *data) {

}

int cache_write(cache_t *cache, int memory_address, size_t bytes_amount, int *data) {

}


cache_t *create_cache(size_t size, size_t e, size_t s) {
}

/* Valida que el input sea correcto
	Abre el archivo
	Crea el cache
	Llama al simulador
	Cierra el archivo
	Libera el cache
*/


int cache_simulator(FILE *tracefile, cache_t *cache) {
	// Parseo las líneas del archivo
	int instruction_pointer, memory_address, data;
	char operation;
	size_t bytes_amount;

	bool errors = false;
	char **strv; // Para guardar los campos de cada línea
	char *line = NULL; // Para que getline se encargue de manejar la memoria
	size_t bytes = 0; // Para que getline se encargue de manejar la memoria
	long int read_bytes;
	FILE *input;
	while ((read_bytes = getline(&line, &bytes, input)) != -1 && !errors) {
		if (line[read_bytes - 1] == '\n') line[--read_bytes] = '\0'; // Piso el \n
		strv = split(line, ' '); // Separo la línea por espacios

		instruction_pointer = (int) strtol(strv[0], NULL, 16);
		operation = strv[1][0];
		memory_address = (int) strtol(strv[2], NULL, 16);
		bytes_amount = (size_t) atoi(strv[3]);
		data = (int) strtol(strv[4], NULL, 16);

		if (operation == 'W') {
			errors = cache_write(cache, memory_address, bytes_amount, &data);
		}
		else if (operation == 'R') {
			errors = cache_read(cache, memory_address, bytes_amount, &data);
		}
		free_strv(strv);
		if (errors) break;
	}
	free(line);
	if (!errors) return 0;
}


int is_power_of_2(int x) {
	return x && !(x & (x - 1));
}

int inputs_are_valid(int a, int b, int c) {
	if (a <= 0 || b <= 0 || c <= 0) return 0;
	return is_power_of_2(a) || is_power_of_2(b) || is_power_of_2(c);
}

int main(int argc, char const *argv[]) {
	// ./cachesim tracefile.xex C E S [ -v n m ]
	if (!(argc == 5 || argc == 8)) {
		fprintf(stderr, "Error: Cantidad de argumentos incorrecta\n");
		exit(1);
	}
	int c = atoi(argv[2]); // tamaño de caché (bytes)
	int e = atoi(argv[3]); // líneas por set
	int s = atoi(argv[4]); // número de sets
	if (inputs_are_valid(c, e, s)) {
		fprintf(stderr, "Error: Argumento incorrecto\n");
		exit(1);
	}
	int n, m;
	if (argc == 8) {
		n = atoi(argv[6]); // líneas por set
		m = atoi(argv[7]); // número de sets
		if (n <= 0 || m <= 0 || n > m) {
			fprintf(stderr, "Error en modo verboso: Argumento incorrecto\n");
			exit(1);
		}
	}

	FILE *tracefile;
	const char *input_file = argv[1];
	tracefile = fopen(input_file, "r");
	if (!tracefile) {
		fprintf(stderr, "Error: Archivo fuente inaccesible");
		return 0;
	}
	cache_t *cache = create_cache(c, e, s);
	int cache_result = cache_simulator(tracefile, cache);
	return 0;
}
