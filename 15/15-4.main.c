#include "15-4.h"

#include <tlpi_hdr.h>

void usageError(const char *name);

int main(int argc, char *argv[]) {
    const char *Mode_chars = "frwx";

    int optchar;
    int mode = 0;
    char *pathname;

    while ((optchar = getopt(argc, argv, Mode_chars)) != -1) {
        if (optchar == '?') {
            usageError(argv[0]);
        }
        switch (optchar) {
            // TODO: Fail if optarg exists.
            case 'f': mode |= F_OK; break;
            case 'r': mode |= R_OK; break;
            case 'w': mode |= W_OK; break;
            case 'x': mode |= X_OK; break;
        }
    }

    // Because F_OK is 0, we just default to F_OK if user does not specify any flag.
    if (optind != argc - 1) {
        usageErr(argv[0]);
    }
    pathname = argv[optind];

    if (access_eid(pathname, mode) == -1) {
        errExit("access_eid");
    }
}

void usageError(const char *name) {
    static const char *usage = "%s -frwx file\n";
    usageErr(usage, name);
}
