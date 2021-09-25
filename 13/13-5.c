#include <fcntl.h>
#include <get_num.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

void write_all(int fd, void *buffer, size_t count);

off_t find_nth_last_line(off_t n, int fd, const struct stat *statbuf, char *buffer);

int main(int argc, char *argv[]) {
    int optchar;
    const char *usage = "%s [ -n num ] file\n";
    off_t num_lines = 10;
    char *file_path;

    while ((optchar = getopt(argc, argv, "n:")) != -1) {
        if (optchar == '?') {
            usageErr(usage, argv[0]);
        }
        if (optchar == 'n') {
            num_lines = getLong(optarg, GN_NONNEG, "num");
        }
    }

    if (optind != argc - 1) {
        usageErr(usage, argv[0]);
    }
    file_path = argv[optind];

    int fd = open(file_path, O_RDONLY);
    if (fd == -1) {
        errExit("open");
    }

    struct stat statbuf;
    if (fstat(fd, &statbuf) == -1) {
        errExit("fstat");
    }

    // TODO: Try memalign.
    char *buffer = malloc(statbuf.st_blksize);
    if (buffer == NULL) {
        errExit("malloc");
    }

    // printf("%ld %ld %ld\n", (long)statbuf.st_size, (long)statbuf.st_blksize, (long)statbuf.st_blocks);
    off_t nth_last_line_position = find_nth_last_line(num_lines, fd, &statbuf, buffer);
    if (nth_last_line_position >= statbuf.st_size) {
        return 0;
    }

    blkcnt_t block_index = nth_last_line_position / statbuf.st_blksize;
    if (lseek(fd, block_index * statbuf.st_blksize, SEEK_SET) == -1) {
        errExit("lseek");
    }

    ssize_t num_read;
    // Usually there will be one block that is not full, if we align our read(),
    // then the first block written to kernel cache will be not full, and
    // all write()s are likely not aligned to memory page boundary. But it
    // should still be better than if we do unaligned read() since RAM is
    // usually faster than disk.
    num_read = read(fd, buffer, statbuf.st_blksize);
    write_all(
        STDOUT_FILENO,
        buffer + nth_last_line_position % statbuf.st_blksize,
        num_read - nth_last_line_position % statbuf.st_blksize);

    while ((num_read = read(fd, buffer, statbuf.st_blksize)) != 0) {
        write_all(STDOUT_FILENO, buffer, num_read);
    }

    free(buffer);
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

off_t find_nth_last_line(off_t n, int fd, const struct stat *statbuf, char *buffer) {
    // According to test with tail(1), the last line may or may not end with newline.
    // We discard the last byte, then find the nth last newline character.
    off_t size = statbuf->st_size - 1;
    if (size <= 0) {
        return 0;
    }
    ssize_t num_read;
    off_t num_newlines = 0;
    for (blkcnt_t block_index = size / statbuf->st_blksize; block_index >= 0;
            --block_index) {
        num_read = pread(
            fd, buffer, statbuf->st_blksize, block_index * statbuf->st_blksize);
        for (ssize_t i = num_read - 1; i >= 0; --i) {
            if (buffer[i] == '\n') {
                num_newlines += 1;
                if (num_newlines == n) {
                    return block_index * statbuf->st_blksize + i + 1;
                }
            }
        }
    }
    return 0;
}
