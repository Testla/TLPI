#include "5-4.h"
#include <assert.h>

void test_and_close_duplicate(int oldfd, int newfd);

int main(void) {
    int fd, newfd;
    char template[] = "/tmp/tmp.XXXXXX";

    fd = mkstemp(template);
    if (fd == -1) {
        errExit("mkstemp");
    }
    unlink(template);

    // Test dup_().

    newfd = dup_(fd);
    if (newfd == -1) {
        errExit("dup_");
    }
    printf("newfd = %d\n", newfd);

    test_and_close_duplicate(fd, newfd);

    // Test dup2_().

    newfd = dup2_(fd, 5);
    if (newfd == -1) {
        errExit("dup2_");
    }
    if (newfd != 5) {
        errExit("Unexpected dup2_ newfd: %d", newfd);
    }
    printf("newfd = %d\n", newfd);

    test_and_close_duplicate(fd, newfd);

    // Cleanup.

    if (close(fd) == -1) {
        errExit("close");
    }
}

void test_and_close_duplicate(int oldfd, int newfd) {
    int offset, offset_new_file;
    int flags, flags_oldfd;

    offset_new_file = lseek(newfd, 1, SEEK_CUR);
    if (offset_new_file == -1) {
        errExit("lseek");
    }

    offset = lseek(oldfd, 0, SEEK_CUR);
    if (offset == -1) {
        errExit("lseek");
    }

    assert(offset == offset_new_file);

    flags = fcntl(oldfd, F_GETFL);
    if (flags == -1) {
        errExit("fcntl F_GETFL");
    }

    flags ^= O_APPEND;
    if (fcntl(newfd, F_SETFL, flags) == -1) {
        errExit("fcntl F_SETFL");
    }

    flags_oldfd = fcntl(oldfd, F_GETFL);
    if (flags_oldfd == -1) {
        errExit("fcntl F_GETFL");
    }

    assert(flags_oldfd == flags);

    if (close(newfd) == -1) {
        errExit("close");
    }
}
