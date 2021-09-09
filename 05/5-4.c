#include "5-4.h"

int dup_(int oldfd);
int dup2_(int oldfd, int newfd);

int dup_(int oldfd) {
    return fcntl(oldfd, F_DUPFD, 0);
}

int dup2_(int oldfd, int newfd) {
    if (oldfd == newfd) {
        if (fcntl(newfd, F_GETFL) == -1) {
            errno = EBADF;
            return -1;
        }
        return oldfd;
    }
    if (fcntl(newfd, F_GETFL) != -1) {
        // Ignore possible error just as the origin dup2().
        close(newfd);
    }
    return fcntl(oldfd, F_DUPFD, newfd);
}
