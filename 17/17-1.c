/*
The problem description does not specify where the acl is. I added one more
command-line for the path from which to read the acl.
Part of this file is copied from acl/acl_view.c.
*/
#include <acl/libacl.h>
#include <sys/acl.h>
#include <sys/stat.h>
#include <tlpi_hdr.h>
#include <ugid_functions.h>

void display_permset(acl_permset_t permset);

int main(int argc, char *argv[]) {
    static const char *usage = "%s u/g identifier path";

    struct stat sb;

    acl_t acl;
    acl_entry_t entry;
    acl_tag_t tag;
    uid_t *uidp, target_uid;
    gid_t *gidp, target_gid;
    acl_permset_t permset;
    int entryId, permVal;

    if (argc != 4) {
        usageErr(usage, argv[0]);
    }

    if (!((argv[1][0] == 'u' || argv[1][0] == 'g') && argv[1][1] == '\0')) {
        usageErr(usage, argv[0]);
    }
    char identifier_type = argv[1][0];

    if (identifier_type == 'u') {
        target_uid = userIdFromName(argv[2]);
    } else {
        target_gid = groupIdFromName(argv[2]);
    }
    if (((identifier_type == 'u') ? target_uid : target_gid) == -1) {
        fatal("No matching user or group found");
    }

    if (stat(argv[3], &sb) == -1) {
        errExit("stat");
    }

    acl = acl_get_file(argv[3], ACL_TYPE_ACCESS);
    if (acl == NULL) {
        errExit("acl_get_file");
    }

    // Not sure if ACL entries are guarateed to be in the order in 17.1, so we
    // save all information that may be necessary.

    Boolean matched = FALSE;
    acl_permset_t matched_permset, other_permset, mask_permset, result;

    for (entryId = ACL_FIRST_ENTRY; ; entryId = ACL_NEXT_ENTRY) {
        if (acl_get_entry(acl, entryId, &entry) != 1)
            break;

        if (acl_get_tag_type(entry, &tag) == -1)
            errExit("acl_get_tag_type");

        if (acl_get_permset(entry, &permset) == -1)
            errExit("acl_get_permset");

        if (tag == ACL_USER_OBJ) {
            if (identifier_type == 'u' && target_uid == sb.st_uid) {
                matched = TRUE;
                matched_permset = permset;
            }
        } else if (tag == ACL_USER) {
            uidp = acl_get_qualifier(entry);
            if (uidp == NULL)
                errExit("acl_get_qualifier");
            if (identifier_type == 'u' && target_uid == *uidp) {
                matched = TRUE;
                matched_permset = permset;
            }
            if (acl_free(uidp) == -1)
                errExit("acl_free");
        } else if (tag == ACL_GROUP_OBJ) {
            if (identifier_type == 'g' && target_gid == sb.st_gid) {
                matched = TRUE;
                matched_permset = permset;
            }
        } else if (tag == ACL_GROUP) {
            gidp = acl_get_qualifier(entry);
            if (gidp == NULL)
                errExit("acl_get_qualifier");
            if (identifier_type == 'g' && target_gid == *gidp) {
                matched = TRUE;
                matched_permset = permset;
            }
            if (acl_free(gidp) == -1)
                errExit("acl_free");
        } else if (tag == ACL_MASK) {
            mask_permset = permset;
        } else if (tag == ACL_OTHER) {
            other_permset = permset;
        }
    }

    // 17.2 ACL Permission-Checking Algorithm

    if (matched) {
        result = matched_permset;
        display_permset(result);
        if (!(identifier_type == 'u' && target_uid == sb.st_uid)) {
            // Apply mask
            acl_perm_t permissions[] = {ACL_READ, ACL_WRITE, ACL_EXECUTE};
            for (int i = 0; i < sizeof(permissions) / sizeof(*permissions); ++i) {
                permVal = acl_get_perm(mask_permset, permissions[i]);
                if (permVal == -1)
                    errExit("acl_get_perm");
                if (permVal == 0) {
                    if (acl_delete_perm(result, permissions[i]) == -1)
                        errExit("acl_delete_perm");
                }
            }
            display_permset(result);
        }
    } else {
        display_permset(other_permset);
    }

    if (acl_free(acl) == -1)
        errExit("acl_free");
}

void display_permset(acl_permset_t permset) {
    int permVal;

    permVal = acl_get_perm(permset, ACL_READ);
    if (permVal == -1)
        errExit("acl_get_perm - ACL_READ");
    printf("%c", (permVal == 1) ? 'r' : '-');
    permVal = acl_get_perm(permset, ACL_WRITE);
    if (permVal == -1)
        errExit("acl_get_perm - ACL_WRITE");
    printf("%c", (permVal == 1) ? 'w' : '-');
    permVal = acl_get_perm(permset, ACL_EXECUTE);
    if (permVal == -1)
        errExit("acl_get_perm - ACL_EXECUTE");
    printf("%c", (permVal == 1) ? 'x' : '-');

    printf("\n");
}
