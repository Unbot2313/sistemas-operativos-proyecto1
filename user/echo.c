#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

    // mostrar por terminal texto, de momento todo sin comillas
    // todo lo que va despues de arg 0 (ese es el comando), es texto de momento no se maneja redirecciones
    // ciclo para mostrar todos los argumentos q son texto
    // ejm: echo(0) hola(1) mundo(2) adios(3) -> hola mundo adios
    for (int i = 1; i < argc; i++) {
        write(1, argv[i], strlen(argv[i]));
        write(1, " ", 1);
    }


}