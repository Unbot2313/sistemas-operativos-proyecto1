#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

#define BUFSIZE 512

static int
contains_pattern(const char *text, const char *pattern)
{
    int i = 0;
    int j;

    if (pattern[0] == 0)
        return 1;

    while (text[i] != 0) {
        j = 0;
        while (pattern[j] != 0 && text[i + j] == pattern[j])
            j++;

        if (pattern[j] == 0)
            return 1;

        i++;
    }

    return 0;
}

static void
print_line_if_match(const char *pattern, const char *filename, int show_filename, char *line)
{
    if (contains_pattern(line, pattern)) {
        if (show_filename)
            printf("%s:", filename);

        printf("%s", line);
    }
}

static void
grep_stream(int fd, const char *pattern, const char *filename, int show_filename)
{
    char buf[BUFSIZE];
    char line[BUFSIZE + 1];
    int n;
    int line_len = 0;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (int i = 0; i < n; i++) {
            char c = buf[i];

            if (line_len < BUFSIZE)
                line[line_len++] = c;

            if (c == '\n') {
                line[line_len] = 0;
                print_line_if_match(pattern, filename, show_filename, line);
                line_len = 0;
            }
        }
    }

    if (line_len > 0) {
        line[line_len] = 0;
        print_line_if_match(pattern, filename, show_filename, line);
        printf("\n");
    }
}

int
main(int argc, char *argv[])
{
    char *pattern;
    int file_index;

    if (argc < 2) {
        printf("Uso: grep <patron> [archivo ...]\n");
        exit(1);
    }

    pattern = argv[1];
    file_index = 2;

    if (argc == 2) {
        grep_stream(0, pattern, "(stdin)", 0);
        exit(0);
    }

    for (int i = file_index; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);

        if (fd < 0) {
            printf("grep: %s: no existe el archivo\n", argv[i]);
            continue;
        }

        grep_stream(fd, pattern, argv[i], argc > 3);
        close(fd);
    }

    exit(0);
}
