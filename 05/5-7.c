#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <tlpi_hdr.h>
#include <unistd.h>

ssize_t readv_(int fd, const struct iovec *iov, int iovcnt);
size_t writev_(int fd, const struct iovec *iov, int iovcnt);

#define TEST_STRING "0123456789"

int main(void) {
    char src[] = TEST_STRING, dest[sizeof(src)];
    struct iovec src_vec[sizeof(src)], dest_vec[sizeof(src)];
    int fd;
    char template[] = "/tmp/tmp.XXXXXX";
    int num_written, num_read;

    // Initialize iovecs to be one-byte buffers.
    for (int i = 0; i < sizeof(src); ++i) {
        src_vec[i].iov_base = &src[i];
        src_vec[i].iov_len = 1;
        dest_vec[i].iov_base = &dest[i];
        dest_vec[i].iov_len = 1;
    }

    fd = mkstemp(template);
    if (fd == -1) {
        errExit("mkstemp");
    }
    // unlink(template);

    num_written = writev_(fd, src_vec, sizeof(src));
    if (num_written == -1) {
        errExit("writev_");
    }
    if (num_written != sizeof(src)) {
        fatal("writev_");
    }

    if (lseek(fd, 0, SEEK_SET) != 0) {
        errExit("lseek");
    }

    num_read = readv_(fd, dest_vec, sizeof(src));
    if (num_read == -1) {
        errExit("readv_");
    }
    if (num_read != sizeof(src)) {
        fatal("readv_");
    }

    assert(memcmp(src, dest, sizeof(src)) == 0);

    if (close(fd) == -1) {
        errExit("close");
    }

    return 0;
}

ssize_t readv_(int fd, const struct iovec *iov, int iovcnt) {
    int total_size = 0;
    char *buffer;
    int num_read;

    for (int i = 0; i < iovcnt; ++i) {
        total_size += iov[i].iov_len;
    }

    buffer = malloc(total_size);

    num_read = read(fd, buffer, total_size);

    for (int i = 0, remain = num_read, copy_size; remain > 0; ++i) {
        copy_size = (remain >= iov[i].iov_len) ? iov[i].iov_len : remain;
        memcpy(iov[i].iov_base, &buffer[total_size - remain], copy_size);
        remain -= copy_size;
    }

    free(buffer);

    return num_read;
}

size_t writev_(int fd, const struct iovec *iov, int iovcnt) {
    int total_size = 0;
    char *buffer, *next;
    int num_written;

    for (int i = 0; i < iovcnt; ++i) {
        total_size += iov[i].iov_len;
    }

    buffer = malloc(total_size);

    next = buffer;
    for (int i = 0; i < iovcnt; ++i) {
        memcpy(next, iov[i].iov_base, iov[i].iov_len);
        next += iov[i].iov_len;
    }

    num_written = write(fd, buffer, total_size);

    free(buffer);

    return num_written;
}
