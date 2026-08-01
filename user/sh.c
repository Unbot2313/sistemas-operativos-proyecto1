#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  static char buf[100];

  while (1) {
    printf("$ ");                 // muestra el prompt
    gets(buf, sizeof(buf));       // lee una línea de input
    if (buf[0] == 0)              // si no hay nada (EOF), termina
      break;
    printf("dijiste: %s", buf);   // por ahora solo repite lo que escribiste
  }

  exit(0);
}

int execute(char *cmd, char **argv){

  int pid = fork();

  if (pid < 0) {
    printf("Error al crear el proceso hijo\n");
    return -1;
  } 

  if (pid == 0) {
    // Proceso hijo
    exec(cmd, argv);
    printf("Error al ejecutar el comando\n");
    exit(-1);
  } else {
    // Proceso padre
    wait(0); // Espera a que el proceso hijo termine
  }

}