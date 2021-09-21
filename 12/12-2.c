// Assumes that each line is of format "<name>:\t<value>\n"
#include <dirent.h>
#include <fcntl.h>
#include <linux/sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tlpi_hdr.h>
#include <ugid_functions.h>
#include <unistd.h>

#define STATUS_FILENAME_LENGTH 24
#ifndef TASK_COMM_LEN
#define TASK_COMM_LEN 16
#endif
#define LINE_BUFFER_LENGTH 20

static const char
    *COMMAND_NAME_FIELD_NAME = "Name",
    *PPID_FIELD_NAME = "PPid";

typedef struct Process_info {
    pid_t pid;
    pid_t ppid;
    char command_name[TASK_COMM_LEN];
    struct Process_info *next;
} Process_info;

typedef struct Tree_node {
    Process_info *process_info;
    struct Tree_node *first_child;
    struct Tree_node *next_sibling;
} Tree_node;

void read_process_info(pid_t pid, Process_info **processes, unsigned long *num_processes);

int compare_tree_node_by_pid(const void *a, const void *b);

Tree_node *binary_search(pid_t pid, Tree_node *nodes, int num);

void print_tree(Tree_node *cur, int depth, Boolean last_sibling[]);

int main(int argc, char *argv[]) {
    if (!(argc == 1 || argc == 3)) {
        usageErr("%s [pid_to_read_first delay_after_first_read]\n", argv[0]);
    }

    pid_t pid_to_read_first = 0;
    unsigned int delay_after_first_read;
    if (argc == 3) {
        pid_to_read_first = getLong(argv[1], GN_GT_0, "pid_to_read_first");
        delay_after_first_read = getInt(argv[2], GN_NONNEG, "delay_after_first_read");
    }

    Process_info *processes = NULL;
    unsigned long num_processes = 0;

    if (pid_to_read_first) {
        read_process_info(pid_to_read_first, &processes, &num_processes);
        sleep(delay_after_first_read);
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

        if (pid == pid_to_read_first) {
            // Already read.
            continue;
        }

        read_process_info(pid, &processes, &num_processes);
    }
    if (errno) {
        errExit("readdir");
    }

    if (closedir(dirp) == -1) {
        errExit("closedir");
    }

    for (Process_info *p = processes; p; p = p->next) {
        printf("%s %ld %ld\n", p->command_name, (long)p->pid, (long)p->ppid);
    }
    puts("");

    Tree_node *nodes = malloc(sizeof(Tree_node) * num_processes);
    int node_index = 0;
    for (Process_info *p = processes; p; p = p->next) {
        nodes[node_index].first_child = NULL;
        nodes[node_index].next_sibling = NULL;
        nodes[node_index].process_info = p;
        node_index += 1;
    }

    qsort(nodes, num_processes, sizeof(*nodes), compare_tree_node_by_pid);

    for (node_index = 0; node_index < num_processes; ++node_index) {
        printf(
            "%s %ld %ld\n",
            nodes[node_index].process_info->command_name,
            (long)(nodes[node_index].process_info->pid),
            (long)(nodes[node_index].process_info->ppid));
    }
    puts("");

    // Scan backward so that process with smaller pid
    // appear earlier in the resulting tree.
    for (node_index = num_processes - 1; node_index > 0; --node_index) {
        Tree_node *parent = binary_search(nodes[node_index].process_info->ppid, nodes, num_processes);
        if (parent == NULL) {
            fprintf(
                stderr,
                "Parent of %ld not found, defaulting to 1\n",
                (long)nodes[node_index].process_info->pid);
            parent = &nodes[0];
        }
        nodes[node_index].next_sibling = parent->first_child;
        parent->first_child = &nodes[node_index];
    }

    Boolean *last_sibling = malloc(sizeof(*last_sibling) * num_processes);
    print_tree(nodes, 0, last_sibling);

    free(last_sibling);
    free(nodes);
    for (Process_info *p = processes, *old; p; ) {
        old = p;
        p = p->next;
        free(old);
    }
}

void read_process_info(pid_t pid, Process_info **processes, unsigned long *num_processes) {
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
            return;
        }
        errExit("fopen");
    }

    Process_info *new_process = malloc(sizeof(Process_info));
    if (new_process == NULL) {
        errExit("malloc");
    }
    new_process->pid = pid;
    new_process->next = *processes;
    *processes = new_process;
    *num_processes += 1;

    char line_buffer[LINE_BUFFER_LENGTH];
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
            strncpy(
                new_process->command_name,
                colon_position + 2,
                sizeof(new_process->command_name));
        } else if (strcmp(line_buffer, PPID_FIELD_NAME) == 0) {
            errno = 0;
            new_process->ppid = strtol(colon_position + 2, NULL, 10);
            if (errno) {
                errExit("strtol");
            }
        }
    }

    if (fclose(file) == EOF) {
        errExit("fclose");
    }
}

int compare_tree_node_by_pid(const void *a, const void *b) {
    return
        ((Tree_node *)a)->process_info->pid
        - ((Tree_node *)b)->process_info->pid;
}

Tree_node *binary_search(pid_t pid, Tree_node *nodes, int num) {
    int l = 0, r = num - 1, mid;
    while (l < r) {
        mid = (l + r) / 2;
        pid_t node_pid = nodes[mid].process_info->pid;
        if (node_pid < pid) {
            l = mid + 1;
        } else if (node_pid == pid) {
            return &nodes[mid];
        } else {
            r = mid;
        }
    }
    return NULL;
}

void print_tree(Tree_node *cur, int depth, Boolean last_sibling[]) {
    for (int i = 1; i < depth; ++i) {
        printf(last_sibling[i] ? "    " : "│   ");
    }
    if (depth > 0) {
        printf(cur->next_sibling ? "├───" : "└───");
    }

    printf("%s(%ld)\n", cur->process_info->command_name, (long)(cur->process_info->pid));

    if (cur->first_child) {
        last_sibling[depth] = (cur->next_sibling == NULL);
        print_tree(cur->first_child, depth + 1, last_sibling);
    }

    if (cur->next_sibling) {
        print_tree(cur->next_sibling, depth, last_sibling);
    }
}
