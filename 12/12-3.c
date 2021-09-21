// Assumes that each line is of format "<name>:\t<value>\n"
#include <dirent.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tlpi_hdr.h>
#include <ugid_functions.h>
#include <unistd.h>

#define STATUS_FILENAME_LENGTH 24
#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif
#define LINE_BUFFER_LENGTH (6 + TASK_COMM_LEN + 1)

static const char *COMMAND_NAME_FIELD_NAME = "Name";

Boolean has_the_file_open(pid_t pid, char *path);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        usageErr("%s opened_path\n", argv[0]);
    }

    DIR *dirp = opendir("/proc/");
    if (dirp == NULL) {
        errExit("opendir");
    }

    struct dirent *dirent;
    while ((errno = 0, dirent = readdir(dirp)) != NULL) {
        if (dirent->d_type != DT_DIR) {
            continue;
        }

        char *endptr;
        errno = 0;
        pid_t pid = strtol(dirent->d_name, &endptr, 10);
        if (errno) {
            errExit("strtol d_name");
        }
        if (*endptr != '\0' || pid <= 0) {
            continue;
        }

        if (!has_the_file_open(pid, argv[1])) {
            continue;
        }

        char status_filename[STATUS_FILENAME_LENGTH];
        int snprintf_result = snprintf(
            status_filename, sizeof(status_filename), "/proc/%ld/status", (long)pid);
        if (snprintf_result < 0) {
            errExit("snprintf");
        } else if (snprintf_result >= sizeof(status_filename)) {
            fatal(
                "stat_filename requires %d bytes but only %d bytes are given",
                snprintf_result + 1, sizeof(status_filename));
        }

        FILE *file = fopen(status_filename, "r");
        if (file == NULL) {
            if (errno == ENOENT) {
                fprintf(stderr, "Skipping pid %ld that does not exist\n", (long)pid);
                continue;
            }
            errExit("fopen");
        }

        char line_buffer[LINE_BUFFER_LENGTH];
        // According to https://man7.org/linux/man-pages/man5/proc.5.html,
        // TASK_COMM_LEN is enough for Name.
        char command_name[TASK_COMM_LEN];
        command_name[0] = '\0';
        // It would be convenient to use getline(),
        // but TLPI's Makefile.inc only requires SUSv3
        // and getline() requires SUSv4.
        while ((line_buffer[sizeof(line_buffer) - 2] = '\0', 
                    fgets(line_buffer, sizeof(line_buffer), file))
                != NULL) {
            // Use the technique in
            // https://github.com/michaelforney/samurai/commit/edeec43d638c826d9e446917f97e95151988e0e0
            // to detect if fgets() has read an incomplete line.
            if (line_buffer[sizeof(line_buffer) - 2]
                    && line_buffer[sizeof(line_buffer) - 2] != '\n') {
                // Skip until the next line.
                // https://stackoverflow.com/a/16108311
                fscanf(file, "%*[^\n]\n");
            }

            char *colon_position = strchr(line_buffer, ':');
            if (colon_position == NULL) {
                // Uncomment this line and try commenting out the fscanf call
                // that skips until the next line
                // to verify that we do handle long lines correctly.
                // fprintf(stderr, "Line %s doesn't contain a colon\n", line_buffer);
                continue;
            }
            *colon_position = '\0';

            if (strcmp(line_buffer, COMMAND_NAME_FIELD_NAME) == 0) {
                char *trailing_newline = strchr(colon_position + 2, '\n');
                if (trailing_newline != NULL) {
                    *trailing_newline = '\0';
                }

                // Do not use scanf family because it may contain whitespace.
                strncpy(command_name, colon_position + 2, sizeof(command_name));
            }
        }

        printf("%ld %s\n", (long)pid, command_name);

        if (fclose(file) == EOF) {
            errExit("fclose");
        }
    }
    if (errno) {
        errExit("readdir");
    }

    if (closedir(dirp) == -1) {
        errExit("closedir");
    }
}

Boolean has_the_file_open(pid_t pid, char *path) {
    char buffer[STATUS_FILENAME_LENGTH * 2];
    int snprintf_result;

    snprintf_result = snprintf(buffer, sizeof(buffer), "/proc/%ld/fd/", (long)pid);
    if (snprintf_result < 0) {
        errExit("snprintf");
    } else if (snprintf_result >= sizeof(buffer)) {
        fatal(
            "fd directory name requires %d bytes but only %d bytes are given",
            snprintf_result + 1, sizeof(buffer));
    }

    DIR *dirp = opendir(buffer);
    if (dirp == NULL) {
        errExit("opendir");
    }

    int path_len = strlen(path);
    // Like the example of readlink(2),
    // we add one to the link size to determine
    // whether the buffer returned by readlink() was truncated.
    char *readlink_buffer = malloc(path_len + 1);
    struct dirent *dirent;
    while ((errno = 0, dirent = readdir(dirp)) != NULL) {
        if (dirent->d_type != DT_LNK) {
            continue;
        }

        snprintf_result = snprintf(buffer, sizeof(buffer), "/proc/%ld/fd/%s", (long)pid, dirent->d_name);
        if (snprintf_result < 0) {
            errExit("snprintf");
        } else if (snprintf_result >= sizeof(buffer)) {
            fatal(
                "fd directory name requires %d bytes but only %d bytes are given",
                snprintf_result + 1, sizeof(buffer));
        }

        ssize_t nbytes = readlink(buffer, readlink_buffer, path_len + 1);
        if (nbytes == path_len && memcmp(readlink_buffer, path, path_len) == 0) {
            return TRUE;
        }
    }
    if (errno) {
        errExit("readdir");
    }

    if (closedir(dirp) == -1) {
        errExit("closedir");
    }

    return FALSE;
}
