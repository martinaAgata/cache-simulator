#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "strutil.h"
#define ADDRESS_SIZE 32
#define ACCESS_PENALTY 100 // Sólo a modo de recordatorio, ver si sirve así o no

// Política de desalojo: LRU

/********************* Estructuras provisorias ********************************/

typedef struct {
	size_t tag;
	size_t index;
	size_t offset;
	size_t operation_index;
} access_data_t;

typedef struct {
	size_t valid;
	size_t dirty;
	size_t tag;
	size_t block;
	size_t last_used;
} line_t;

typedef struct {
	line_t **lines;
	size_t size; // size = E
} set_t;

typedef struct {
	size_t C;
	size_t E;
	size_t S;
	size_t block_bytes;
	set_t **sets;
	size_t offset_bits; // Ver después si dejar todos estos acá
	size_t index_bits;
	size_t tag_bits;
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

unsigned int log_2(unsigned int x) { // Ver si usar esto o no
	unsigned int y = 0 ;
	while ( x >>= 1 ) y++;
	return y;
}

/*
	Recibe la caché y el patrón de acceso, y carga la línea en la caché,
	devuelve NULL en caso de error, caso contrario devuelve la línea cargada.
 */
line_t *load_line(cache_t *cache, access_data_t *access_data) {
	line_t* line = NULL;
	line_t *invalid_line = NULL;
	line_t *least_used_line = cache->sets[access_data->index]->lines[0];
	// Busco si el tag está cargado en la línea pero es invalido
	for (size_t i = 0; i < cache->E ; i++) {
		line = cache->sets[access_data->index]->lines[i];
		if ((line->tag == access_data->tag) && !(line->valid)) { // redundante la ultima cond
			// Si el tag estaba pero era invalida, cambio la línea a válida porque la "cargué"
			line->valid = 1;
			// ¿no se debe actualizar last_used?
			return line;
		}
		// Voy guardando la linea menos utilizada
		if (line->last_used >= least_used_line->last_used) {
			least_used_line = line;
		}
		// Guardo alguna línea invalida que haya encontrado
		if (!line->valid) {
			invalid_line = line;
		}
	}
	// Recorrí todas las líneas y no encontré el tag
	if (invalid_line) line = invalid_line; // Reemplazo una línea invalida
	else line = least_used_line; // Reemplazo la línea menos utilizada

	if (!line) return NULL;

	line->valid = 1;
	line->tag = access_data->tag;
	line->last_used = access_data->operation_index;
	return line;
}

/*
	Recibe la caché y el patrón de acceso, y busca entre los elementos de la caché,
	si hay una coincidencia devuelve la linea, si no devuelve NULL
 */
line_t *check_for_hit(cache_t *cache, access_data_t *access_data) {
	for (size_t i = 0; i < cache->E ; i++) {
		if ((cache->sets[access_data->index]->lines[i]->valid) &&
			(cache->sets[access_data->index]->lines[i]->tag == access_data->tag)) {
			return cache->sets[access_data->index]->lines[i];
		}
	}
	return NULL;
}

int cache_write(cache_t *cache, access_data_t *data, size_t bytes_amount, stats_t *stats) {
	line_t *hit_line = check_for_hit(cache, data);
	if (hit_line) { // Write hit
		// Actualizar dato en bloque (al actualizar dejar B bytes)
		// Actualizo el acceso para la política de desalojo
		hit_line->last_used = data->operation_index;
		// Actualizar estadísticas
		stats->stores++;
		hit_line->dirty = 1;
	} else { // Write miss
		// SI HAY DESALOJO Y DIRTY EN 1 --> PONGO DIRTY BIT EN 0
		line_t *loaded_line = load_line(cache, data);
		// Actualizar las estadísticas
		stats->loads++; // Cualquier miss incrementa bytes read
		// Solo si es dirty
		stats->wmiss++;
	}
	return 0;
}

int cache_read(cache_t *cache, access_data_t *data, size_t bytes_amount, stats_t *stats) {
	line_t *hit_line = check_for_hit(cache, data);
	if (hit_line) { // Read hit
		// Actualizo el acceso para la política de desalojo
		hit_line->last_used = data->operation_index; // ¿solo se actualiza si hubo hit?
		// Actualizar estadísticas
		stats->loads++;

	} else { // Read miss
		// Actualizar el dirty read
		// Ver si hay que desalojar (usando LRU)
		cache_write(cache, data, bytes_amount, stats);
		// Actualizar las estadísticas
		stats->rmiss++;
	}
	return 0; // Ver qué devolver
}

cache_t *create_cache(size_t c, size_t e, size_t s) {
	cache_t *cache = calloc(1, sizeof(cache_t));
	cache->C = c;
	cache->E = e;
	cache->S = s;
	cache->block_bytes = c / (e*s);
	cache->offset_bits = cache->C / (cache->S * cache->E);
	cache->index_bits = log_2(cache->S);
	cache->tag_bits = ADDRESS_SIZE - cache->index_bits - cache->offset_bits;

	size_t i, j;
	for (i = 0; i < s; i++) {
		cache->sets[i] = calloc(1, sizeof(set_t));
		if (cache->sets[i] == NULL) return NULL;
		cache->sets[i]->size = e;
		for (j = 0; j < e; j++) {
			cache->sets[i]->lines[j] = calloc(1, sizeof(line_t));
			if (cache->sets[i]->lines[j] == NULL) return NULL;
			// Si calloc() no funcionó habría que liberar el resto de la memoria
			// y no devolver NULL directo, ¿no?
		}
	}
	return cache;
	// Liberar memoria del resto de los sets o líneas si un malloc() falla
}

/*
	Extrae de una dirección de 32 bits: tag, set index y block offset teniendo
	en cuenta los valores de E, S y C de la caché actual, y devuelve un objeto
	access_data_t que almacena esos datos.
 */
access_data_t *get_access_data(cache_t *cache, size_t op_index, int memory_address) {

	size_t offset = ((1 << cache->offset_bits) - 1) & memory_address;
	size_t index = (((1 << cache->index_bits) - 1) & memory_address) >> cache->offset_bits;
	size_t tag = (((1 << cache->tag_bits) - 1) & memory_address) >> (cache->offset_bits + cache->index_bits);

	access_data_t *data = malloc(sizeof(access_data_t));
	if (!data) return NULL;

	data->tag = tag;
	data->index = index;
	data->offset = offset;
	data->operation_index = op_index;
	return data;
}

int cache_simulator(FILE *tracefile, cache_t *cache, bool verbose, int n, int m) {
	// Parseo las líneas del archivo
	int memory_address;
	char operation;
	size_t bytes_amount;
	stats_t *stats = create_stats();

	bool errors = false;
	int lines_read = -1; // Para cuando haya que imprimir con verbose
	char **strv; // Para guardar los campos de cada línea
	char *line = NULL; // Para que getline se encargue de manejar la memoria
	size_t bytes = 0; // Para que getline se encargue de manejar la memoria
	long read_bytes;
	size_t op_index = 0;
	access_data_t *access_data = NULL;
	while ((read_bytes = getline(&line, &bytes, tracefile)) != -1 && !errors) {
		lines_read++;
		if (line[read_bytes - 1] == '\n') line[--read_bytes] = '\0'; // Piso el \n
		strv = split(line, ' ');

		operation = strv[1][0];
		memory_address = (int) strtol(strv[2], NULL, 16);
		bytes_amount = (size_t) atoi(strv[3]);

		access_data = get_access_data(cache, op_index, memory_address);
		op_index++;
		// Actualizar el objeto stats

 		if (operation == 'R') {
			errors = cache_read(cache, access_data, bytes_amount, stats);
		} else if (operation == 'W') {
			errors = cache_write(cache, access_data, bytes_amount, stats);
		}

		if (verbose) printf("%s\n", "Estoy en modo verboso así que imprimo lo que hice");

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
	int n = 0;
	int m = 0; // Si no se inicializan n y m la línea 242 puede romper
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
		fprintf(stderr, "Error: Archivo fuente inaccesible\n");
		return 0;
	}
	cache_t *cache = create_cache(c, e, s);
	cache_simulator(tracefile, cache, verbose, n, m);
	return 0;
}
