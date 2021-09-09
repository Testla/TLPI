// The data appear at the end of the file,
// because O_APPEND means "Writes are always appended to end of file".

#include <fcntl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

void write_all(int fd, void *buffer, size_t count);

int main(int argc, char *argv[]) {
    int fd;

    if (argc != 2) {
        usageErr("%s filename\n", argv[0]);
    }

    fd = open(argv[1], O_WRONLY | O_APPEND);
    if (fd == -1) {
        errExit("open");
    }

    if (lseek(fd, 0, SEEK_SET) == -1) {
        errExit("seek");
    }

    write_all(fd, "x", 1);

    if (close(fd) == -1) {
        errExit("close");
    }
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
