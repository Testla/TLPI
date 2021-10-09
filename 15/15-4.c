#include "15-4.h"

#include <unistd.h>

static int swap_reids(void) {
    // Guess we don't need to handle supplementary groups?
    if (setreuid(geteuid(), getuid()) == -1) {
        return -1;
    }
    return setregid(getegid(), getgid());
}

int access_eid(const char *pathname, int mode) {
    // To be able to set the real ID back to the initial one, we need to
    // "save" it to effective ID.
    // From page 176:
    // ... on BSD, setreuid() and setregid() permitted a process to drop and
    // regain privilege by swapping the values of the real and effective IDs
    // back and forth.
    // If we do this, the saved set-user/group-ID will be set to the initial
    // effective ID, which may be unwanted side effect, the only way I can think
    // of to avoid it is to use the nonstandard getresuid() family.
    if (swap_reids() == -1) {
        return -1;
    }
    int result = access(pathname, mode);
    if (swap_reids() == -1) {
        return -1;
    }
    return result;
}
