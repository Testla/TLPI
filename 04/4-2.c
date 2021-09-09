// Guess I only have to implement functionality of
// copying content of one file to another,
// without that tons of options and directory support?

#include <fcntl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

#define BUFFER_SIZE 100

void write_all(int fd, const void *buffer, size_t count);

int main(int argc, char *const argv[]) {
    int src_fd, dest_fd;
    char buffer[BUFFER_SIZE];
    ssize_t num_read;
    // 0 for \0, 1 for other.
    int current_type;
    int seek_amount;

    if (argc != 3) {
        usageErr("%s src dest", argv[0]);
    }

    src_fd = open(argv[1], O_RDONLY);
    if (src_fd == -1) {
        errExit("open src");
    }
    dest_fd = open(
        argv[2],
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dest_fd == -1) {
        errExit("open dest");
    }

    num_read = read(src_fd, buffer, BUFFER_SIZE);
    if (num_read > 0) {
        // Initialize current_type.
        current_type = !!buffer[0];
    }
    seek_amount = 0;
    while (num_read != 0) {
        if (num_read == -1) {
            errExit("read");
        }

        // Try to reduce number of syscalls as much as possible.
        for (const char *begin = buffer, *end = buffer; end < buffer + num_read; begin = end) {
            // Find and process the next segment of bytes of same type.
            while (end < buffer + num_read && current_type == !!(*end)) {
                end += 1;
            }

            if (current_type) {
                // When the first byte is of different type,
                // we will have end == begin,
                // we don't write zero byte in this case.
                if (end != begin) {
                    write_all(dest_fd, begin, end - begin);
                }
            } else {
                seek_amount += end - begin;
                if (end != buffer + num_read) {
                    lseek(dest_fd, seek_amount, SEEK_CUR);
                    seek_amount = 0;
                }
            }

            if (end != buffer + num_read) {
                current_type = !!(*end);
            }
        }

        num_read = read(src_fd, buffer, BUFFER_SIZE);
    }

    // Write trailing null bytes.
    if (seek_amount) {
        lseek(dest_fd, seek_amount - 1, SEEK_CUR);
        write_all(dest_fd, "\0", 1);
    }

    if (close(src_fd) == -1) {
        errExit("close src");
    }
    if (close(dest_fd) == -1) {
        errExit("close dest");
    }
}

void write_all(int fd, const void *buffer, size_t count) {
    int num_written = 0, result;
    while (num_written < count) {
        result = write(fd, (char *)buffer + num_written, count - num_written);
        if (result == -1) {
            errExit("write to %d", fd);
        }
        num_written += result;
    }
}
