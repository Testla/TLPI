// Assumes that each line is of format "<name>:\t<value>\n"
#include <dirent.h>
#include <fcntl.h>
#include <get_num.h>
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
// Intentionally set the length of line buffer to a just-big-enough value
// to test if we handle the case that fgets reads an incomplete line correctly.
// "Name:\t" is 6 bytes long, one more byte for newline.
#define LINE_BUFFER_LENGTH (6 + TASK_COMM_LEN + 1)

static const char *COMMAND_NAME_FIELD_NAME = "Name", *UID_FIELD_NAME = "Uid";

int main(int argc, char *argv[]) {
    if (!(argc == 2 || argc == 4)) {
        usageErr("%s username [pid_to_delay delay_before_open]\n", argv[0]);
    }

    char *user = argv[1];
    uid_t uid = userIdFromName(user);
    printf("Programs run by %s(%ld):\n", user, (long)uid);

    pid_t pid_to_delay = 0;
    unsigned int delay_before_open;
    if (argc == 4) {
        pid_to_delay = getLong(argv[2], GN_GT_0, "pid_to_delay");
        delay_before_open = getInt(argv[3], GN_NONNEG, "delay_before_open");
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

        char status_filename[STATUS_FILENAME_LENGTH];
        int snprintf_result = snprintf(
            status_filename, sizeof(status_filename), "/proc/%d/status", pid);
        if (snprintf_result < 0) {
            errExit("snprintf");
        } else if (snprintf_result >= sizeof(status_filename)) {
            fatal(
                "stat_filename requires %d bytes but only %d bytes are given",
                snprintf_result + 1, sizeof(status_filename));
        }

        if (pid_to_delay == pid) {
            sleep(delay_before_open);
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
        long ruid = -1;
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
            } else if (strcmp(line_buffer, UID_FIELD_NAME) == 0) {
                errno = 0;
                ruid = strtol(colon_position + 2, NULL, 10);
                if (errno) {
                    errExit("strtol");
                }

                if (ruid != uid) {
                    break;
                }
            }
        }

        if (ruid == uid && command_name[0]) {
            printf("%ld %s\n", (long)pid, command_name);
        }

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
