CFLAGS := -ggdb3 -O2 -Wall -Wextra -std=c11 # o "-g" en lugar de "-ggdb3"
CPPFLAGS := -D_POSIX_C_SOURCE=200809L # Esto es para usar POSIX.2008-1 como default

cachesim: cachesim.o strutil.o
