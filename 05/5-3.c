// O_APPEND is atomic but lseek followed by write isn't.
// So in the latter case two processes may seek and write to the same location,
// making the resulting file size smaller.

#include <fcntl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

static const char *Byte_to_write = "a";
void write_all(int fd, void *buffer, size_t count);

int main(int argc, char *argv[]) {
    long num_bytes;
    int use_append;
    int fd;

    if (argc < 3 || argc > 4) {
        usageErr("%s filename num-bytes [x]\n", argv[0]);
    }

    num_bytes = getLong(argv[2], GN_NONNEG, "num-bytes");
    use_append = (argc == 3);

    fd = open(
        argv[1],
        O_WRONLY | O_CREAT | (use_append ? O_APPEND : 0),
        S_IRUSR | S_IWUSR);
    if (fd == -1) {
        errExit("open");
    }

    for (int i = 0; i < num_bytes; ++i) {
        if (!use_append) {
            if (lseek(fd, 0, SEEK_END) == -1) {
                errExit("seek");
            }
        }
        write_all(fd, (void *)Byte_to_write, 1);
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
