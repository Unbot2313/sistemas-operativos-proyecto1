#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
    int pid = fork();

    char buf[512];

    if (pid < 0) {
        printf("Error en fork\n");
        exit(1);
    }

    if (pid == 0) {

        int fd = open(argv[1], O_RDONLY);

        int read_valor = read(fd, buf, 512);

        printf("read valor: %d\n", read_valor);

        close(fd);

        write(1, buf, read_valor);

        exit(0);
    }

    wait(0);


    exit(0);
}