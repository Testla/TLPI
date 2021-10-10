#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <unistd.h>

static const char *Attribute_letters = "acDijAdtsSTu";
static const int Attribute_values[] = {
    FS_APPEND_FL,
    FS_COMPR_FL,
    FS_DIRSYNC_FL,
    FS_IMMUTABLE_FL,
    FS_JOURNAL_DATA_FL,
    FS_NOATIME_FL,
    FS_NODUMP_FL,
    FS_NOTAIL_FL,
    FS_SECRM_FL,
    FS_SYNC_FL,
    FS_TOPDIR_FL,
    FS_UNRM_FL
};

int main(int argc, char *argv[]) {
    if (argc < 3) {
        usageErr(
            "%s mode files...\n"
            "Format of mode is +-=[aAcCdDeijPsStTu]\n",
            argv[0]);
    }

    char operator = argv[1][0];
    if (!(operator == '+' || operator == '-' || operator == '=')) {
        fatal("Unknown operator %s", argv[1][0]);
    }

    int attributes = 0;
    for (int i = 1; argv[1][i]; ++i) {
        for (int j = 0; ; ++j) {
            if (Attribute_letters[j] == '\0') {
                fatal("Unknown attribute letter %c", argv[1][i]);
            }
            if (argv[1][i] == Attribute_letters[j]) {
                attributes |= Attribute_values[j];
                break;
            }
        }
    }

    for (int i = 2; i < argc; ++i) {
        int fd, old_attr, new_attr;

        if ((fd = open(argv[i], O_RDONLY)) == -1) {
            errExit("open %s", argv[i]);
        }

        if (operator == '+' || operator == '-') {
            if (ioctl(fd, FS_IOC_GETFLAGS, &old_attr) == -1) {
                errExit("ioctl");
            }
        }
        switch(operator) {
            case '+': new_attr = old_attr | attributes; break;
            case '-': new_attr = old_attr & (~attributes); break;
            case '=': new_attr = attributes; break;
        }
        if (ioctl(fd, FS_IOC_SETFLAGS, &new_attr) == -1) {
            errExit("ioctl");
        }

        if (close(fd) == -1) {
            errExit("close");
        }
    }
}
