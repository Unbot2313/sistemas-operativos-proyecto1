#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    // mostrar por terminal contenido de un archivo basico

    // arvg 1 es el nombre del archivo a leer
    int fd = open(argv[1], O_RDONLY);

    char buf[512];
    int read_valor;

    // read devuelve el numero de bytes leidos
    // si devuelve 0 es porque se acabo el archivo
    // se usa un ciclo para leer archivos grandes por limites del buffer

    while ((read_valor = read(fd, buf, 512)) > 0) {
        write(1, buf, read_valor);
    }

    close(fd);

    exit(0);
}