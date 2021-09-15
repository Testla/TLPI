#include "9-3.h"

#include <limits.h>
#include <pwd.h>
#include <tlpi_hdr.h>
#include <ugid_functions.h>

#define SG_SIZE (NGROUPS_MAX + 1)

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usageErr("%s <username>\n", argv[0]);
    }

    char *user = argv[1];
    struct passwd *pwd;
    gid_t gid;

    if ((pwd = getpwnam(user)) == NULL) {
        errExit("getpwnam");
    }
    gid = pwd->pw_gid;

    if (initgroups_(user, gid) == -1) {
        errExit("initgroups_");
    }

    gid_t grouplist[SG_SIZE];
    int num_groups;
    char *p;

    if ((num_groups = getgroups(SG_SIZE, grouplist)) == -1) {
        errExit("getgroups");
    }

    printf("groups of %s:", user);
    for (int i = 0; i < num_groups; ++i) {
        p = groupNameFromId(grouplist[i]);
        printf(" %s(%ld)", (p == NULL) ? "???" : p, (long) grouplist[i]);
    }
    puts("");

    return 0;
}
