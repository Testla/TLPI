#include <fcntl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

#define BUFFER_SIZE 100

void write_all(int fd, void *buffer, size_t count);

int main(int argc, char *argv[]) {
    int optchar;
    int append = 0;
    char *filename;
    int fd;
    char buffer[BUFFER_SIZE];
    ssize_t num_read;
    const char *usage = "%s [-a] <file>\n";

    while ((optchar = getopt(argc, argv, "a")) != -1) {
        if (optchar == '?') {
            usageErr(usage, argv[0]);
        }
        if (optchar == 'a') {
            append = 1;
        }
    }

    if (optind != argc - 1) {
        usageErr(usage, argv[0]);
    }
    filename = argv[optind];

    fd = open(
        filename,
        O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC),
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd == -1) {
        errExit("open");
    }

    while ((num_read = read(STDIN_FILENO, buffer, BUFFER_SIZE)) != 0) {
        if (num_read == -1) {
            errExit("read");
        }
        write_all(STDOUT_FILENO, buffer, num_read);
        write_all(fd, buffer, num_read);
    }

    if (close(fd) == -1) {
        errExit("close");
    }

    return 0;
}

void write_all(int fd, void *buffer, size_t count) {
    int num_written = 0, result;
    while (num_written < count) {
        result = write(fd, (char *)buffer + num_written, count - num_written);
        if (result == -1) {
            errExit("write to %d", fd);
        }
        num_written += result;
    }
}
