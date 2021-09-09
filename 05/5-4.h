#include <fcntl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

int dup_(int oldfd);
int dup2_(int oldfd, int newfd);
