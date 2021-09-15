#include "9-3.h"

#include <limits.h>
#include <string.h>

int initgroups_(const char *user, gid_t group) {
    struct group *grp;
    gid_t groups[NGROUPS_MAX + 1];
    int num_groups;

    num_groups = 0;
    while ((grp = getgrent()) != NULL) {
        for (char **mem = grp->gr_mem; *mem; ++mem) {
            if (strcmp(*mem, user) == 0) {
                groups[num_groups++] = grp->gr_gid;
                break;
            }
        }
    }
    endgrent();

    groups[num_groups++] = group;

    if (setgroups(num_groups, groups) == -1) {
        return -1;
    }

    return 0;
}
