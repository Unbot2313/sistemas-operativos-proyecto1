// user/sh.c
// Shell para xv6: comandos simples, argumentos, redireccion (< >),
// tuberias simples y multiples (|), y comando interno exit.

#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define MAXCMDS 10   // maximo de comandos encadenados por pipes
#define MAXARGS 16   // maximo de argumentos por comando

// Ejecuta UN comando (sin pipe), con redireccion opcional de entrada/salida.
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

// Ejecuta una cadena de N comandos conectados por pipes.
// argvs[i]  -> argv del comando i
// ins[i]    -> archivo de entrada (<) del comando i, o 0
// outs[i]   -> archivo de salida (>) del comando i, o 0
// ncmds     -> cantidad de comandos en la cadena
int
RunPipeline(char *argvs[MAXCMDS][MAXARGS], char *ins[MAXCMDS], char *outs[MAXCMDS], int ncmds)
{
  int i, pid, fd[2];
  int in_fd = 0; // 0 significa "todavia no viene de ningun pipe"

  for (i = 0; i < ncmds; i++) {
    int has_next = (i < ncmds - 1);

    if (has_next && pipe(fd) < 0) {
      printf("Error al crear el pipe\n");
      return -1;
    }

    pid = fork();
    if (pid < 0) {
      printf("Error al crear el proceso hijo\n");
      return -1;
    }

    if (pid == 0) {
      // ENTRADA: del pipe anterior, o de archivo (<), o la normal (consola)
      if (in_fd != 0) {
        close(0);
        dup(in_fd);
        close(in_fd);
      } else if (ins[i] != 0) {
        int fdin = open(ins[i], O_RDONLY);
        if (fdin < 0) {
          printf("Error al abrir la entrada %s\n", ins[i]);
          exit(1);
        }
        close(0);
        dup(fdin);
        close(fdin);
      }

      // SALIDA: al siguiente pipe, o a archivo (>), o la normal (consola)
      if (has_next) {
        close(1);
        dup(fd[1]);
        close(fd[1]);
        close(fd[0]);
      } else if (outs[i] != 0) {
        int fdout = open(outs[i], O_WRONLY | O_CREATE | O_TRUNC);
        if (fdout < 0) {
          printf("Error al abrir la salida %s\n", outs[i]);
          exit(1);
        }
        close(1);
        dup(fdout);
        close(fdout);
      }

      exec(argvs[i][0], argvs[i]);
      printf("Error al ejecutar el comando\n");
      exit(-1);
    }

    // Padre: cierra lo que ya no necesita y "avanza" el extremo de lectura
    if (in_fd != 0)
      close(in_fd);
    if (has_next) {
      close(fd[1]);
      in_fd = fd[0];
    }
  }

  for (i = 0; i < ncmds; i++)
    wait(0);

  return 0;
}

int
main(void)
{
  static char buf[100];

  while (1) {
    printf("$ ");
    gets(buf, sizeof(buf));
    if (buf[0] == 0) {
      break; // EOF: termina el shell
    }

    char *argvs[MAXCMDS][MAXARGS];
    int argc[MAXCMDS];
    char *ins[MAXCMDS];
    char *outs[MAXCMDS];
    int ncmds = 1;
    int cur = 0;
    int i;

    for (i = 0; i < MAXCMDS; i++) {
      argc[i] = 0;
      ins[i] = 0;
      outs[i] = 0;
    }

    char *p = buf;

    while (*p) {
      while (*p == ' ' || *p == '\t' || *p == '\n') {
        *p++ = 0;
      }
      if (*p == 0)
        break;

      char *token = p;
      while (*p && *p != ' ' && *p != '\t' && *p != '\n') {
        p++;
      }
      if (*p != 0)
        *p++ = 0;

      if (strcmp(token, "|") == 0) {
        cur++;
        if (cur >= MAXCMDS) {
          printf("Demasiados comandos en la tuberia\n");
          cur = MAXCMDS - 1;
        }
        ncmds = cur + 1;
        continue;
      }

      if (strcmp(token, "<") == 0 || strcmp(token, ">") == 0) {
        int is_input = (strcmp(token, "<") == 0);

        while (*p == ' ' || *p == '\t' || *p == '\n')
          *p++ = 0;
        if (*p == 0) {
          printf("Falta el archivo despues de %s\n", token);
          argc[cur] = 0;
          break;
        }

        char *file = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n')
          p++;
        if (*p != 0)
          *p++ = 0;

        if (is_input)
          ins[cur] = file;
        else
          outs[cur] = file;
        continue;
      }

      if (argc[cur] < MAXARGS - 1) {
        argvs[cur][argc[cur]++] = token;
      }
    }

    for (i = 0; i < ncmds; i++) {
      argvs[i][argc[i]] = 0;
    }

    if (ncmds == 1) {
      if (argc[0] == 0)
        continue; // linea vacia o solo espacios

      if (strcmp(argvs[0][0], "exit") == 0)
        exit(0);

      RunCommand(argvs[0][0], argvs[0], ins[0], outs[0]);
    } else {
      int valid = 1;
      for (i = 0; i < ncmds; i++) {
        if (argc[i] == 0)
          valid = 0;
      }

      if (!valid) {
        printf("Comando invalido en la tuberia\n");
      } else {
        if (strcmp(argvs[0][0], "exit") == 0)
          exit(0);

        RunPipeline(argvs, ins, outs, ncmds);
      }
    }
  } // cierra el while(1)

  exit(0);
}