#include <fcntl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int fd1, fd2, fd3;
    char *file;

    if (argc != 2) {
        usageErr("%s <file>\n", argv[0]);
    }
    file = argv[1];

    fd1 = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    fd2 = dup(fd1);
    fd3 = open(file, O_RDWR);
    write(fd1, "Hello,", 6);
    // "Hello,". When file is opened, its offset is 0.
    write(fd2, "world", 6);
    // "Hello,world\0". fd2 is a duplicate of fd1, so they share the same offset.
    lseek(fd2, 0, SEEK_SET);
    write(fd1, "HELLO,", 6);
    // "HELLO,world\0". fd2 is a duplicate of fd1, so they share the same offset.
    write(fd3, "Gidday", 6);
    // "Giddayworld\0". fd3 refers to a different open file description.

    if (close(fd1) == -1 || close(fd2) == -1 || close(fd3) == -1) {
        errExit("close");
    }
}
