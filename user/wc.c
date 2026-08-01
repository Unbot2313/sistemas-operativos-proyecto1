#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

    // wc = word count
    // $ echo esto es un ejemplo > texto.txt
    // $ cat texto.txt
    // esto es un ejemplo
    // $ wc texto.txt
    // 1 4 19 texto.txt

    // los bytes se pueden obtener con read
    // las palabras es contando los espacios
    // y lineas es contando los saltos de línea?

    // arvg 1 es el nombre del archivo a leer
    int fd = open(argv[1], O_RDONLY);

    // devuelve -1 si ocurre un error como no encontrar el archivo
    if (fd < 0) {
        printf("Error al abrir el archivo %s\n", argv[1]);
        exit(1);
    }

    char buf[512];
    int read_valor;

    // read devuelve el numero de bytes leidos
    // si devuelve 0 es porque se acabo el archivo
    // se usa un ciclo para leer archivos grandes por limites del buffer
    int total_bytes = 0;
    int total_words = 0;
    int total_lines = 0;
    while ((read_valor = read(fd, buf, 512)) > 0) {
        // contar lineas, palabras y bytes
        total_bytes += read_valor;
        for (int i = 0; i < read_valor; i++) {
            if (buf[i] == ' ') {
                total_words++;
            }
            if (buf[i] == '\n') {
                total_lines++;
            }
        }
    }

    close(fd);

    printf("%d %d %d %s\n", total_lines, total_words, total_bytes, argv[1]);
}