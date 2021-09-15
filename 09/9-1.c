#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <sys/fsuid.h>
#include <tlpi_hdr.h>
#include <unistd.h>

int check_credentials(uid_t eruid, uid_t eeuid, uid_t esuid, uid_t efsid);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usageErr("%s <a/b/c/d/e>\n", argv[0]);
    }

    // Allow user other than uid 1000 to run the test.
    uid_t ruid = getuid();
    if (ruid == 0 || ruid == 2000 || ruid == 3000) {
        fatal(
            "Do no run this test as root, uid 2000 or uid 3000"
            " because it may affect the result.");
    }

    if (!check_credentials(ruid, 0, 0, 0)) {
        fatal("Initial user IDs incorrect");
    }

    switch(argv[1][0]) {
        case 'a':
            // All user IDs are set for privileged process.
            setuid(2000);
            assert(check_credentials(2000, 2000, 2000, 2000));
            break;
        case 'b':
            // Saved set-user-ID is also set because effective user ID being set
            // does not equal to the previous real ID.
            setreuid(-1, 2000);
            assert(check_credentials(ruid, 2000, 2000, 2000));
            break;
        case 'c':
            // Effective ID is set and file-system ID follows.
            seteuid(2000);
            assert(check_credentials(ruid, 2000, 0, 2000));
            break;
        case 'd':
            setfsuid(2000);
            assert(check_credentials(ruid, 0, 0, 2000));
            break;
        case 'e':
            setresuid(-1, 2000, 3000);
            assert(check_credentials(ruid, 2000, 3000, 2000));
            break;
        default:
            usageErr("%s <a/b/c/d/e>\n", argv[0]);
    }
    return 0;
}

int check_credentials(uid_t eruid, uid_t eeuid, uid_t esuid, uid_t efsuid) {
    uid_t ruid, euid, suid, fsuid;
    if (getresuid(&ruid, &euid, &suid) == -1) {
        errExit("getresuid");
    }
    fsuid = setfsuid(0);
    setfsuid(fsuid);

    int result = (
        (ruid == eruid)
        && (euid == eeuid)
        && (suid == esuid)
        && (fsuid == efsuid));

    if (!result) {
        printf("%ld %ld %ld %ld\n", (long)ruid, (long)euid, (long)suid, (long)fsuid);
    }

    return result;
}
