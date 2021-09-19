// https://www.man7.org/linux/man-pages/man3/setenv.3.html
#include "6-3.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

static int check_name(const char *name) {
	return name && name[0] && (strchr(name, '=') == NULL);
}

int setenv_(const char *name, const char *value, int overwrite) {
    int name_len, value_len;
    char *new_entry;

	if (!check_name(name)) {
		errno = EINVAL;
		return -1;
	}

    if (overwrite == 0 && getenv(name) != NULL) {
        return 0;
    }

    name_len = strlen(name);
    value_len = strlen(value);

    new_entry = malloc(name_len + 1 + value_len + 1);
    if (new_entry == NULL) {
		errno = ENOMEM;
        return -1;
    }

    memcpy(new_entry, name, name_len);
    new_entry[name_len] = '=';
    memcpy(new_entry + name_len + 1, value, value_len);
    new_entry[name_len + 1 + value_len] = '\0';

    if (putenv(new_entry)) {
        return -1;
    }

    return 0;
}

int unsetenv_(const char *name) {
	if (!check_name(name)) {
		errno = EINVAL;
		return -1;
	}

    int name_len = strlen(name);
    char **end = environ;
    for (char **ep = environ; *ep; ++ep) {
        if (memcmp(*ep, name, name_len) == 0 && (*ep)[name_len] == '=') {
            continue;
        }
        *(end++) = *ep;
    }
    *end = NULL;
    return 0;
}
