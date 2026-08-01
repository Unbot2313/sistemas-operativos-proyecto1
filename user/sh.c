#include "kernel/types.h"
#include "user/user.h"

int
RunCommand(char *cmd, char **argv)
{
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

  return 0;
}

int
main(void)
{
  static char buf[100];

  while (1) {
    printf("$ ");                 // muestra el prompt
    gets(buf, sizeof(buf));       // lee una línea de input
    if (buf[0] == 0){
      break;  // si no hay nada (EOF), termina
    }              

    int argc = 0;
    char *argv[16];
    char *p = buf;

    while (*p) {
      while (*p == ' ' || *p == '\t') {
        *p++ = 0;
      }
      if (*p == 0)
        break;
      if (argc < 15) {
        argv[argc++] = p;
      }
      while (*p && *p != ' ' && *p != '\t') {
        p++;
      }
    }
    argv[argc] = 0;

    if (argc > 0) {
      RunCommand(argv[0], argv); // ejecuta el comando
    }

  }

  exit(0);
}

