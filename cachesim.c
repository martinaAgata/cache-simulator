#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "strutil.h"
#define address_size 32

// Las direcciones de memoria son siempre de 32 bits.
// La política que desalojo, que es siempre least-recently used (LRU)
// La penalty por accesos a memoria es siempre 100 ciclos

// #include <stdint.h>
// typedef uint32_t memory_address; QUIZAS SIRVA

/********************* Estructuras provisorias ********************************/

typedef struct {
	size_t tag;
	size_t index;
	size_t offset;
} access_data_t;

typedef struct {
	size_t valid;
	size_t dirty;
	size_t tag;
	size_t block;
} line_t;

typedef struct {
	line_t **lines;
	size_t size; // size = E
} set_t;

typedef struct {
	size_t c;
	size_t e;
	size_t s;
	size_t block_bytes;
	set_t **sets;
} cache_t;

/*
	Estructura para almacenar las estádisticas y no tener que estar
	pasando todas esas variables de una función a otra.
*/
typedef struct {
	size_t loads;
	size_t stores;
	size_t rmiss;
	size_t wmiss;
	size_t dirty_rmiss;
	size_t dirty_wmiss;
	size_t bytes_read;
	size_t bytes_written;
	size_t read_time;
	size_t write_time;
} stats_t;

/******************************************************************************/

stats_t *create_stats(void) {
	stats_t *stats = malloc(sizeof(stats_t));
	stats->loads = 0;
	stats->stores = 0;
	stats->rmiss = 0;
	stats->wmiss = 0;
	stats->dirty_rmiss = 0;
	stats->dirty_wmiss = 0;
	stats->bytes_read = 0;
	stats->bytes_written = 0;
	stats->read_time = 0;
	stats->write_time = 0;
	return stats;
}


/*
	Recibe la caché y un tag, y busca entre todos los elementos de la caché,
	si hay una coincidencia devuelve la linea, si no devuelve NULL
*/
line_t *check_for_match(cache_t *cache, size_t tag) {
	// Algoritmo pésimo pero podemos mejorarlo después, por ahora es más o menos es útil.
	size_t i, j;
	for (i = 0; i < cache->s ; i++) {
		for (j = 0; j < cache->s ; j++) {
			if (cache->sets[i]->lines[j]->tag == tag) {
				return cache->sets[i]->lines[j];
			}
		}
	}
	return NULL;
}

/*
	S = 2 ^ s
	B = 2 ^ b
	t = m - s - b
	m = t + s + b
*/
access_data_t *get_access_data(int memory_address) {
	// ¿Se podrá hacer sin memoria dinámica? Tipo, ¿tendrá algún beneficio?
	access_data_t *data = malloc(sizeof(access_data_t));
	if (!data) return NULL;
	// ...
	// data->tag = ;
	// data->index = ;
	// data->offset = ;
	return data;
}

int cache_read(cache_t *cache, access_data_t *data, size_t bytes_amount, stats_t *stats) {
	// Coming Soon
	return 0;
}

int cache_write(cache_t *cache, access_data_t *data, size_t bytes_amount, stats_t *stats) {
	// Coming Soon
	return 0;
}

cache_t *create_cache(size_t c, size_t e, size_t s) {
	cache_t *cache = malloc(sizeof(cache_t));
	cache->c = c;
	cache->e = e;
	cache->s = s;
	cache->block_bytes = c / (e*s);

	size_t i, j;
	for (i = 0; i < s; i++) {
		cache->sets[i] = malloc(sizeof(set_t));
		if (cache->sets[i] == NULL) return NULL;
		cache->sets[i]->size = e;
		for (j = 0; j < e; j++) {
			cache->sets[i]->lines[j] = malloc(sizeof(line_t));
			if (cache->sets[i]->lines[j] == NULL) return NULL;
			cache->sets[i]->lines[j]->valid = 0;
			cache->sets[i]->lines[j]->dirty = 0;
			// Ver cómo distribuir los tags en las líneas y qué
			// guardar en el bloque de comienzo (¿nada en todo?)
		}
	}
	return cache;
	// Liberar memoria del resto de los sets o líneas si un malloc() falla
}

int cache_simulator(FILE *tracefile, cache_t *cache, bool verbose, int n, int m) {
	// Parseo las líneas del archivo
	int instruction_pointer, memory_address;
	char operation;
	size_t bytes_amount;
	stats_t *stats = create_stats();

	bool errors = false;
	int lines_read = -1; // Para cuando haya que imprimir con verbose
	char **strv; // Para guardar los campos de cada línea
	char *line = NULL; // Para que getline se encargue de manejar la memoria
	size_t bytes = 0; // Para que getline se encargue de manejar la memoria
	long read_bytes;
	access_data_t *access_data = NULL;
	while ((read_bytes = getline(&line, &bytes, tracefile)) != -1 && !errors) {
		lines_read++;
		if (line[read_bytes - 1] == '\n') line[--read_bytes] = '\0'; // Piso el \n
		strv = split(line, ' ');

		// instruction_pointer = (int) strtol(strv[0], NULL, 16);
		operation = strv[1][0];
		memory_address = (int) strtol(strv[2], NULL, 16);
		bytes_amount = (size_t) atoi(strv[3]);
		access_data = get_access_data(memory_address);

		// Actualizar el objeto stats

		if (operation == 'W') {
			errors = cache_write(cache, access_data, bytes_amount, stats);
		}
		else if (operation == 'R') {
			errors = cache_read(cache, access_data, bytes_amount, stats);
		}

		if (verbose && (lines_read >= n || lines_read <= m) ) {
			printf("%s\n", "Estoy en modo verboso así que imprimo lo que hice");
		}

		free_strv(strv);
		if (errors) break;
	}
	free(line);
	if (!errors) return 0;
	return -1;
}

int is_power_of_2(int x) {
	return x && !(x & (x - 1));
}

int inputs_are_valid(int a, int b, int c) {
	if (a <= 0 || b <= 0 || c <= 0) return 0;
	return is_power_of_2(a) && is_power_of_2(b) && is_power_of_2(c);
}

/*
	Valida input, abre el archivo, crea el cache, llama al simulador
	cierra el archivo y libera el cache
*/
int main(int argc, char const *argv[]) { // ./cachesim tracefile.xex C E S -v n m

	if (!(argc == 5 || argc == 8)) {
		fprintf(stderr, "Error: Cantidad de argumentos incorrecta\n");
		exit(1);
	}

	int c = atoi(argv[2]); // Tamaño de caché en bytes
	int e = atoi(argv[3]); // Líneas por set
	int s = atoi(argv[4]); // Número de sets

	if (inputs_are_valid(c, e, s)) {
		fprintf(stderr, "Error: Argumento incorrecto\n");
		exit(1);
	}

	bool verbose = false;
	int n = 0; // Después vemos qué hacer con esto, el tema es que si no se
	int m = 0; // inicializan la línea 242 puede romper
	if (argc == 8) {
		if (strcmp(argv[5], "-v") == 0) {
			n = atoi(argv[6]);
			m = atoi(argv[7]);
			verbose = true;
		}
		if (n <= 0 || m <= 0 || n > m) {
			fprintf(stderr, "Error: Argumento incorrecto\n");
			exit(1);
		}
	}

	const char *input_file = argv[1];
	FILE *tracefile = fopen(input_file, "r");
	if (!tracefile) {
		fprintf(stderr, "Error: Archivo fuente inaccesible");
		return 0;
	}
	cache_t *cache = create_cache(c, e, s);
	int cache_result = cache_simulator(tracefile, cache, verbose, n, m);
	return 0;
}
