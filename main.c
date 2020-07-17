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
    //Parseo las líneas del archivo
    int instruction_pointer;
    char operation;
    int memory_address;
    size_t bytes_amount;
    int data;

    bool errors = false;
    char** strv;//para guardar los campos de cada línea
    char* line = NULL;//para que getline se encargue de manejar la memoria
    size_t bytes = 0;//para que getline se encargue de manejar la memoria
    long int read_bytes;

    while((read_bytes = getline(&line, &bytes, input)) != -1 && !errors){
       if (line[read_bytes - 1] == '\n') line[--read_bytes] = '\0'; //Piso el \n
       strv = split(line, ' ');//separo la línea por espacios

       instruction_pointer = (int)strtol(strv[0], NULL, 16);
       operation = strv[1];
       memory_address = (int)strtol(strv[2], NULL, 16);
       bytes_amount = (size_t)atoi(strv[3]);
       data = (int)strtol(strv[4], NULL, 16);

       if (operation == 'W'){
           errors = cache_write(cache, memory_address, bytes_amount, &data);
       }
       else if (operation == 'R'){
           errors = cache_read(cache, memory_address, bytes_amount, &data);
       }
       if(errors) break;
       free_strv(strv);
    }
    free(line);
    if (!errors) return 0;
}
