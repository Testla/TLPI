#include "6-3.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <tlpi_hdr.h>

extern char **environ;

static const char *name = "hello", *initial_value = "word", *modified_value = "world";
// To have multiple definitions of an environment variable,
// we use putenv() to get our own strings into the environment list,
// and then modify the name.
static char volatile_entry[] = "hallo=world!";

static void printenv() {
	for (char **ep = environ; *ep; ++ep) {
		puts(*ep);
	}
}

static int countenv(const char *name) {
	int count = 0;
    int name_len = strlen(name);
	for (char **ep = environ; *ep; ++ep) {
        if (memcmp(*ep, name, name_len) == 0 && (*ep)[name_len] == '=') {
			count += 1;
		}
	}
    return count;
}

int main(void) {
    if (clearenv()) {
        errExit("clearenv");
    }

	assert(setenv_(NULL, "", 0) == -1 && errno == EINVAL);
	assert(setenv_("", "", 0) == -1 && errno == EINVAL);
	assert(setenv_("1+1=2", "", 0) == -1 && errno == EINVAL);

	assert(setenv_(name, initial_value, 0) == 0);
	assert(strcmp(getenv(name), initial_value) == 0);

	assert(setenv_(name, modified_value, 0) == 0);
	assert(strcmp(getenv(name), initial_value) == 0);

	assert(setenv_(name, modified_value, 1) == 0);
	assert(strcmp(getenv(name), modified_value) == 0);

	assert(putenv(volatile_entry) == 0);
	assert(strcmp(getenv("hallo"), "world!") == 0);

	volatile_entry[1] = 'e';
	puts("env before unset");
	printenv();

	assert(countenv(name) == 2);

	assert(unsetenv_(name) == 0);
	puts("env after unset");
	printenv();
	assert(countenv(name) == 0);
}
