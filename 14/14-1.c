#include <fcntl.h>
#include <get_num.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <tlpi_hdr.h>
#include <unistd.h>

#define MAX_NF 1000000

int rand_below(int x);

void shuffle(void *array, size_t num, size_t size);

int compare_int(const void *a, const void *b);

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 4) {
        usageErr("%s NF dest_dir [sort_before_create]\n", argv[0]);
    }

    int num_files = getInt(argv[1], GN_GT_0, "NF");
    char *dest_dir = argv[2];
    // Excluding trailing slash.
    int dest_dir_len = strlen(dest_dir);
    if (dest_dir[dest_dir_len - 1] == '/') {
        dest_dir_len -= 1;
    }
    Boolean sort_before_create = (argc == 4);

    // To avoid collision of random generated numbers, we first generate all
    // six-digit numbers, then shuffle them. Because reject rate of the new
    // rand_below implementation should be quite low with x = 1000000, this
    // process should be near constant-time.
    srand(time(NULL));
    int *numbers_in_filename = malloc(sizeof(int) * MAX_NF);
    if (numbers_in_filename == NULL) {
        errExit("malloc");
    }
    for (int i = 0; i < MAX_NF; ++i) {
        numbers_in_filename[i] = i;
    }
    shuffle(numbers_in_filename, MAX_NF, sizeof(int));

    int filename_length = dest_dir_len + 1 + 1 + 6 + 1;
    char *filename = malloc(filename_length);
    if (filename == NULL) {
        errExit("malloc");
    }
    strcpy(filename, dest_dir);

    if (sort_before_create) {
        qsort(
            numbers_in_filename,
            sizeof(*numbers_in_filename), num_files, compare_int);
    }

    for (int i = 0; i < num_files; ++i) {
        int snprintf_result = snprintf(
            filename + dest_dir_len,
            filename_length - dest_dir_len,
            "/x%06d",
            numbers_in_filename[i]);
        if (snprintf_result < 0) {
            errExit("snprintf");
        } else if (snprintf_result >= filename_length) {
            fatal(
                "stat_filename requires %d bytes but only %d bytes are given",
                snprintf_result + 1, filename_length);
        }

        int fd = open(filename, O_WRONLY | O_CREAT | O_EXCL);
        if (fd == -1) {
            errExit("open");
        }
        if (write(fd, "x", 1) != 1) {
            errExit("write");
        }
        if (close(fd) == -1) {
            errExit("close");
        }
    }

    if (!sort_before_create) {
        qsort(
            numbers_in_filename,
            sizeof(*numbers_in_filename), num_files, compare_int);
    }

    for (int i = 0; i < num_files; ++i) {
        int snprintf_result = snprintf(
            filename + dest_dir_len,
            filename_length - dest_dir_len,
            "/x%06d",
            numbers_in_filename[i]);
        if (snprintf_result < 0) {
            errExit("snprintf");
        } else if (snprintf_result >= filename_length) {
            fatal(
                "stat_filename requires %d bytes but only %d bytes are given",
                snprintf_result + 1, filename_length);
        }

        unlink(filename);
    }

    free(filename);
    free(numbers_in_filename);
    return 0;
}

int rand_below(int x) {
    // Try to obtain uniform distribution by resampling.
    int power = 1;
    uint64_t product = RAND_MAX, r;
    while (UINT64_MAX / product > RAND_MAX) {
        product *= RAND_MAX;
        power += 1;
    }
    while (true) {
        r = 0;
        for (int i = 0; i < power; ++i) {
            r = r * RAND_MAX + rand();
        }
        if (r < UINT64_MAX / x * x) {
            return r % x;
        }
    }
}

void shuffle(void *array, size_t num, size_t size) {
    void *buffer = malloc(size);
    if (buffer == NULL) {
        errExit("malloc");
    }

    for (size_t i = num - 1; i > 0; --i) {
        size_t j = rand_below(i + 1);
        memcpy(buffer,                   (char *)array + i * size, size);
        memcpy((char *)array + i * size, (char *)array + j * size, size);
        memcpy((char *)array + j * size, buffer,                   size);
    }

    free(buffer);
}

int compare_int(const void *a, const void *b) {
    return *(int *)a - *(int *)b;
}
