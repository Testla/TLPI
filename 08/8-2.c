#include "8-2.h"

#include <string.h>

struct passwd *getpwnam_(const char *name) {
    struct passwd *pwd;
    while ((pwd = getpwent()) != NULL) {
        if (strcmp(pwd->pw_name, name) == 0) {
            break;
        }
    }
    endpwent();
    return pwd;
}
