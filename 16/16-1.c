#include <sys/xattr.h>
#include <tlpi_hdr.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        usageErr("%s filename name value\n", argv[0]);
    }

    const char *filename = argv[1], *name = argv[2], *value = argv[3];

    if (setxattr(filename, name, value, strlen(value), 0) == -1) {
        errExit("setxattr");
    }

    return 0;
}
