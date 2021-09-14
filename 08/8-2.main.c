#include "8-2.h"

#include <assert.h>

int main(void) {
    struct passwd *pwd;
    pwd = getpwnam_("root");
    assert(pwd->pw_uid == 0);
    return 0;
}
