#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// incluir clase

cache_t *create_cache(size_t size, size_t asoc, size_t sets) {
}

/* Valida que el input sea correcto
	Abre el archivo
	Crea el cache
	Llama al simulador
	Cierra el archivo
	Libera el cache
*/
int number_is_valid(unsigned int x) {
	return x && !(x & (x - 1));
}

int main(int argc, char const *argv[]) {
	// ./cachesim tracefile.xex C E S [ -v n m ]
	if (argc != 4 || argc != 7) {
		fprintf(stderr, "Cantidad de argumentos incorrecta\n");
		exit(1);
	}
	int c = atoi(argv[1]); // el tamaño de la caché C, en bytes
	int e = atoi(argv[2]); // la asociatividad de la caché, E
	int s = atoi(argv[3]); // el número de sets de la caché, S
	if (c <= 0 || e <= 0 || s <= 0) { // FALTA CHEQUEAR SI ADEMAS SON POWER OF 2
		fprintf(stderr, "Argumento incorrecto\n");
		exit(1);
	}

	FILE* tracefile;
	const char* input_file = argv[1];
	tracefile = fopen(input_file, "r");
	if(!tracefile){
		fprintf( stderr, "Error: archivo fuente inaccesible");
		return 0;
	}
	cache_simulator(tracefile, cache);
	return 0;
}

int cache_simulator(FILE* tracefile, cache_t* cache){
	// Parseo las líneas del archivo
	int instruction_pointer, memory_address, data;
	char operation;
	size_t bytes_amount;

	bool errors = false;
	char** strv; // Para guardar los campos de cada línea
	char* line = NULL; // Para que getline se encargue de manejar la memoria
	size_t bytes = 0; // Para que getline se encargue de manejar la memoria
	long int read_bytes;

	while ((read_bytes = getline(&line, &bytes, input)) != -1 && !errors) {
		if (line[read_bytes - 1] == '\n') line[--read_bytes] = '\0'; // Piso el \n
		strv = split(line, ' '); // Separo la línea por espacios

		instruction_pointer = (int) strtol(strv[0], NULL, 16);
		operation = strv[1];
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
