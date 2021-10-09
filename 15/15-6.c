#include <sys/stat.h>
#include <tlpi_hdr.h>

void change_mode(char *path);

int main(int argc, char *argv[]) {
    if (argc == 1) {
        usageErr("%s file [file ...]\n", argv[0]);
    }
    for (int i = 1; i < argc; ++i) {
        change_mode(argv[i]);
    }
    return 0;
}

void change_mode(char *path) {
    static struct stat sb;
    if (stat(path, &sb) == -1) {
        errExit("stat");
    }

    Boolean should_set_execute = FALSE;
    static const mode_t execute_bits = S_IXUSR | S_IXGRP | S_IXOTH;
    if (S_ISDIR(sb.st_mode)) {
        should_set_execute = TRUE;
    } else if (S_ISREG(sb.st_mode)) {
        should_set_execute = (sb.st_mode & execute_bits);
    } else {
        fatal("%s is not a directory or file", path);
    }

    mode_t new_mode = sb.st_mode;
    new_mode |= (S_IRUSR | S_IRGRP | S_IROTH);
    if (should_set_execute) {
        new_mode |= execute_bits;
    }

    if (chmod(path, new_mode) == -1) {
        errExit("chmod");
    }
}
