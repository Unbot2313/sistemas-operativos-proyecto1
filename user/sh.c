#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int
RunCommand(char *cmd, char **argv, char *input_file, char *output_file)
{
  int pid = fork();

  if (pid < 0) {
    printf("Error al crear el proceso hijo\n");
    return -1;
  } 

  if (pid == 0) {
    int fd;

    if (input_file != 0) {
      fd = open(input_file, O_RDONLY);
      if (fd < 0) {
        printf("Error al abrir la entrada %s\n", input_file);
        exit(1);
      }
      close(0);
      dup(fd);
      close(fd);
    }

    if (output_file != 0) {
      fd = open(output_file, O_WRONLY | O_CREATE | O_TRUNC);
      if (fd < 0) {
        printf("Error al abrir la salida %s\n", output_file);
        exit(1);
      }
      close(1);
      dup(fd);
      close(fd);
    }

    exec(cmd, argv);
    printf("Error al ejecutar el comando\n");
    exit(-1);
  } else {
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
    char *input_file = 0;
    char *output_file = 0;
    char *p = buf;

    while (*p) {
      while (*p == ' ' || *p == '\t' || *p == '\n') {
        *p++ = 0;
      }
      if (*p == 0)
        break;

      char *token = p;
      if (argc < 15) {
        argv[argc++] = token;
      }
      while (*p && *p != ' ' && *p != '\t' && *p != '\n') {
        p++;
      }

      if (*p != 0)
        *p++ = 0;

      if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
        char **file = strcmp(token, "<") == 0 ? &input_file : &output_file;

        while (*p == ' ' || *p == '\t' || *p == '\n')
          *p++ = 0;
        if (*p == 0) {
          printf("Falta el archivo despues de %s\n", token);
          argc = 0;
          break;
        }

        *file = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n')
          p++;
        if (*p != 0)
          *p++ = 0;
        argc--;
      }
    }
    argv[argc] = 0;

    if (argc > 0) {
      if (strcmp(argv[0], "exit") == 0) {
        exit(0);   // termina el shell
      }
      RunCommand(argv[0], argv, input_file, output_file);
    }

  }

  exit(0);
}

