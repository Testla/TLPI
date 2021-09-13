#include "7-2.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <tlpi_hdr.h>
#include <unistd.h>

#define NUM_BLOCK 200
#define BLOCK_SIZE 1024

void swap(void **a, void **b);

int rand_below(int x);

int main(void) {
    void *ptr[NUM_BLOCK];
    void *initial_break;

    initial_break = sbrk(0);
    if (initial_break == (void *)-1) {
        errExit("sbrk");
    }
    printf("program break before allocation %p\n", initial_break);

    // The second time I run sbrk(0), the returned value will be 0x21000 larger.

    initial_break = sbrk(0);
    if (initial_break == (void *)-1) {
        errExit("sbrk");
    }
    printf("program break before allocation %p\n", initial_break);

    long page_size = sysconf(_SC_PAGESIZE);
    if (page_size == -1) {
        errExit("get page_size");
    }
    assert((intptr_t)initial_break % page_size == 0);

    for (int i = 0; i < NUM_BLOCK; ++i) {
        ptr[i] = malloc_(BLOCK_SIZE);
        if (ptr[i] == NULL) {
            errExit("malloc_");
        }
    }
    printf("program break after allocation %p\n", sbrk(0));

    for (int i = 0; i < NUM_BLOCK; ++i) {
        free_(ptr[i]);
    }
    printf("program break after free %p\n", sbrk(0));
    assert(sbrk(0) == initial_break);

    // Free from back to front.

    for (int i = 0; i < NUM_BLOCK; ++i) {
        ptr[i] = malloc_(BLOCK_SIZE);
        if (ptr[i] == NULL) {
            errExit("malloc_");
        }
    }
    printf("program break after allocation %p\n", sbrk(0));

    for (int i = NUM_BLOCK - 1; i >= 0; --i) {
        free_(ptr[i]);
    }
    printf("program break after free %p\n", sbrk(0));
    assert(sbrk(0) == initial_break);

    // Random block size, free half in random order,
    // allocate again and free all in random order.

    srand(time(NULL));
    for (int i = 0; i < NUM_BLOCK; ++i) {
        ptr[i] = malloc_(rand_below(BLOCK_SIZE + 1));
        if (ptr[i] == NULL) {
            errExit("malloc_");
        }
    }
    printf("program break after allocation %p\n", sbrk(0));

    for (int i = 1; i < NUM_BLOCK; ++i) {
        swap(ptr[i], ptr[rand_below(i + 1)]);
    }

    for (int i = 0; i < NUM_BLOCK / 2; ++i) {
        free_(ptr[i]);
    }
    for (int i = 0; i < NUM_BLOCK / 2; ++i) {
        ptr[i] = malloc_(rand_below(BLOCK_SIZE + 1));
        if (ptr[i] == NULL) {
            errExit("malloc_");
        }
    }

    for (int i = 1; i < NUM_BLOCK; ++i) {
        swap(ptr[i], ptr[rand_below(i + 1)]);
    }

    for (int i = 0; i < NUM_BLOCK; ++i) {
        free_(ptr[i]);
    }
    printf("program break after free %p\n", sbrk(0));
    assert(sbrk(0) == initial_break);

    return 0;
}

void swap(void **a, void **b) {
    void *t = *a;
    *a = *b;
    *b = t;
}

int rand_below(int x) {
    // Try to obtain uniform distribution by resampling.
    int r;
    while (true) {
        r = rand();
        if (r < RAND_MAX / x * x) {
            return r % x;
        }
    }
}
