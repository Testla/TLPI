#include <assert.h>
#include <stdio.h>
#include <tlpi_hdr.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    uid_t oruid = getuid(), oeuid = geteuid();

    printf("%s oruid=%ld oeuid=%ld\n", argv[0], (long) oruid, (long) oeuid);
    if (oruid == oeuid) {
        fatal("The real user ID and effective user ID should be different.");
    }

    // a) Suspend and resume the set-user-ID identity

    if (seteuid(oruid) == -1) {
        errExit("seteuid");
    }
    assert(geteuid() == oruid);

    if (seteuid(oeuid) == -1) {
        errExit("seteuid");
    }
    assert(geteuid() == oeuid);

    // b) Permanently drop the set-user-ID identity

    if (setreuid(oruid, oruid) == -1) {
        errExit("setreuid");
    }
    assert(seteuid(oeuid) == -1 && errno == EPERM);

    return 0;
}
