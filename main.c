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

  //Tomo la ruta relativa o completa del primer archivo pasado
  const char* file_path = argv[1];
  //Lo abro en modo lectura, guardando el file_descriptor
  int trace_file = open(file_path, O_RDONLY);
  //Finalizo la ejecución si no falló al abrir el archivo
  if (trace_file == -1){
    if (strcmp("EISDIR", strerror(errno)) == 0){
      fprintf( stderr, "Error: el archivo '%s' es un directorio\n", file_path);
      return 3;
    }
    fprintf( stderr, "Error: archivo '%s' inaccesible\n", file_path);
    return 2;
  }
	cache_simulator(trace_file, cache);
	return 0;
}

int cache_simulator(char const* filename, cache_t* cache){


}
